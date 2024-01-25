#include "HookSetup.h"

#include "MyInterface.h"

// 声明了来自 ScriptCore.cpp (位于私有文件目录) 的函数
extern uint8 GRegisterNative( int32 NativeBytecodeIndex, const FNativeFuncPtr& Func );
// 创建 字节码数组的索引
enum
{
	ECustomByteCode = EX_Max - 1
};

static bool bHookNative = false;

// 将来注入到蓝图函数的字节码
// 第一个字节码指向我们自定义的函数 , 然后返回;  数量为3可能是3字节指令 ?
static uint8 CustomByteCode[] = {(uint8)ECustomByteCode, EX_Return, EX_Nothing};

// 注入的函数地址
static FNativeFuncPtr NativeFunc = nullptr;

// 用来标记被注入函数的新名称
#define OLD_FUNC_PREFIX TEXT("OLD_FUNC_PREFIX_")

// 模板黑科技,用来实现私有成员的访问
#define ACCESS_PRIVATE_FIELD(Class, Type, Member) \
template <typename Class, Type Class::* M> \
struct AccessPrivate##Class##Member { \
friend Type Class::* Private##Class##Member() { return M; } \
};\
Type Class::* Private##Class##Member(); \
template struct AccessPrivate##Class##Member<Class, &Class::Member>



// typedef void (*FNativeFuncPtr)(UObject* Context, FFrame& TheStack, RESULT_DECL);
// HOOK 执行函数
static void CustomNativeFunc(UObject* Context, FFrame& TheStack, RESULT_DECL)
{
	UFunction* cur_func = TheStack.Node;
	FString name = cur_func->GetName();

	// 获取被HOOK的函数
	FName old_func_name = FName(*(OLD_FUNC_PREFIX + name));
	UFunction* old_func = Context->GetClass()->FindFunctionByName(old_func_name);
	UE_LOG(LogTemp, Warning, TEXT("hooked func called , find old func [%s] result : [%s]"),*old_func_name.ToString(),old_func?TEXT("TRUE"):TEXT("FALSE"));
	// 调用,这里用的无参数调用;
	Context->ProcessEvent(old_func,nullptr);
	UE_LOG(LogTemp, Warning, TEXT("stack function ptr : %p | old function ptr : %p "),cur_func,old_func);
	
	
	UE_LOG(LogTemp, Warning, TEXT(" hooked func called , origin func name: %s"),*TheStack.Node->GetName());
}

HookSetup::HookSetup()
{
	// 只需要执行一次,注册我们自定义函数到字节码数组
	if (!bHookNative)
	{
		UE_LOG(LogTemp, Warning, TEXT("set native func ptr + register native to ECustomByteCode(255) "));
		NativeFunc = &CustomNativeFunc;
		GRegisterNative(ECustomByteCode,NativeFunc);
		bHookNative = true;
	}

	// 添加对UObject的监听
	id = FMath::RandRange(0,100);
	UE_LOG(LogTemp, Warning, TEXT("[HookSetup->HookSetup]%d"),id);
	GUObjectArray.AddUObjectCreateListener(this);
	GUObjectArray.AddUObjectDeleteListener(this);

}


HookSetup::~HookSetup()
{
	UE_LOG(LogTemp, Warning, TEXT("[HookSetup->~HookSetup]%d"),id);
	GUObjectArray.RemoveUObjectCreateListener(this);
	GUObjectArray.RemoveUObjectDeleteListener(this);
}

ACCESS_PRIVATE_FIELD(FProperty, int, Offset_Internal);


void HookSetup::NotifyUObjectCreated(const UObjectBase* Object, int32 Index)
{
	UObjectBaseUtility* obj = (UObjectBaseUtility*)Object;

	if (!obj->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		UClass* cls = obj->GetClass();
		
		// filter the right object
		if (cls->IsChildOf<UPackage>() || cls->IsChildOf<UClass>())
		{
			return;
		}

		// check implement interface ?
		static UClass* interfaceClass = UMyInterface::StaticClass();
		if (!cls->ImplementsInterface(interfaceClass))
		{
			return;
		}
		if (IsInGameThread())
		{
			UE_LOG(LogTemp, Warning, TEXT("create: %s | %d "),*obj->GetName(),Index);

			// 此时只有在 蓝图中定义或者实现的函数,才能 Find 成功;
			// 可能是调用时机太早,此时无法获取cpp定义的函数;
			TArray<FName> HookFuncList = {
				TEXT("ReceiveBeginPlay") , TEXT("CF_01") , TEXT("CF_02") , TEXT("BP_01") , TEXT("BP_02") ,  TEXT("BP_03")
			};

			for (FName & it : HookFuncList)
			{
				UFunction* func = cls->FindFunctionByName(it,EIncludeSuperFlag::ExcludeSuper);

				if (!func)
				{
					UE_LOG(LogTemp, Warning, TEXT("[NotifyUObjectCreated] cant find func [%s]"),*it.ToString());
					continue;
				}

				FName old_func_name = FName(*(OLD_FUNC_PREFIX + func->GetName()));

				// 这里将蓝图的 BeginPlay 函数进行了拷贝,并且注册成了 OLD_FUNC_PREFIX_ReceiveBeginPlay 函数
				// 这样可以在虚拟机那里进行调用
				// 如果不进行拷贝,两者函数地址一样,会造成同一函数的死循环调用;
				FObjectDuplicationParameters Parameters(func,cls);
				Parameters.DestName = old_func_name;
				Parameters.InternalFlagMask &= ~EInternalObjectFlags::Native;

				// 使用 StaticDuplicateObjectEx 拷贝了 UFunction
				UFunction* old_func = Cast<UFunction>(StaticDuplicateObjectEx(Parameters));
				old_func->PropertyLink = CastField<FProperty>(old_func->ChildProperties);
				old_func->PropertiesSize = func->PropertiesSize;
				old_func->MinAlignment = func->MinAlignment;
				static FArchive Ar;
				static auto PropertyOffsetPtr = PrivateFPropertyOffset_Internal();
				
				for (TFieldIterator<FProperty> srcIt(func),dstIt(old_func); srcIt && dstIt ; ++srcIt, ++dstIt)
				{
					FProperty* scrProperty = *srcIt;
					FProperty* dstProperty = *dstIt;
					
					dstProperty->Link(Ar);
					dstProperty->RepIndex = scrProperty->RepIndex;
					// 这里使用上面的函数模板黑魔法,修改了 FProperty 的私有变量
					dstProperty->*PropertyOffsetPtr = scrProperty->GetOffset_ForInternal();
					dstProperty->PropertyLinkNext = CastField<FProperty>(dstProperty->Next);
				}
				// 注册到当前类的 FunctionMap
				cls->AddFunctionToFunctionMap(old_func,old_func_name);
				

				
				if (func->HasAnyFunctionFlags(FUNC_Native))
				{
					func->SetNativeFunc(NativeFunc);
					UE_LOG(LogTemp, Warning, TEXT("[%s] hook in c++"),*it.ToString());
					continue;
				}


				// 注入字节码
				if (func->HasAnyFunctionFlags(FUNC_BlueprintEvent))
				{
					func->Script.Insert(CustomByteCode,3,0);
					UE_LOG(LogTemp, Warning, TEXT("[%s] hook in bp"),*it.ToString());
				}
			} 
		}
	}
	
}



void HookSetup::NotifyUObjectDeleted(const UObjectBase* Object, int32 Index)
{
	
}

void HookSetup::OnUObjectArrayShutdown()
{
	UE_LOG(LogTemp, Warning, TEXT("[HookSetup->OnUObjectArrayShutdown]"));
}


#undef OLD_FUNC_PREFIX