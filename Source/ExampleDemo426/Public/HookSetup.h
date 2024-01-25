#pragma once

class HookSetup : public FUObjectArray::FUObjectCreateListener, public FUObjectArray::FUObjectDeleteListener
{
	
public:
	HookSetup();
	virtual ~HookSetup() override;
	
	virtual void NotifyUObjectCreated(const UObjectBase* Object, int32 Index) override;
	virtual void NotifyUObjectDeleted(const UObjectBase* Object, int32 Index) override;

	virtual void OnUObjectArrayShutdown() override;


private:
	int32 id = -1;
};
