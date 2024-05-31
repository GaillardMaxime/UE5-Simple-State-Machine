// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "StateMachineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnterStateSignature, FGameplayTag, StateTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExitStateSignature, FGameplayTag, StateTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTickStateSignature, float, DeltaTime, FGameplayTag, StateTag);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STATE_MACHINE_API UStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	/// The initial state of the state machine. 
	UPROPERTY(EditAnywhere, Category = "State Machine")
	FGameplayTag InitialState;
	
	/// The current state of the state machine.
	UPROPERTY()
	FGameplayTag CurrentState;
	
	/// The array of accepted states. If the state machine tries to change to a state
	/// that isn't contain in the accepted state, it won't work 
	UPROPERTY(EditAnywhere, Category = "State Machine")
	TArray<FGameplayTag> AcceptedStates;

	/// If True, enables the debug of the plugin.
	UPROPERTY(EditAnywhere, Category = "State Machine")
	bool bUseDebug;
	
	/// The key for the debug log of the current state.
	UPROPERTY(EditAnywhere, Category = "State Machine", meta = (EditCondition="bUseDebug", EditConditionHides))
	int8 DebugKey;

	

public:	
	/// Sets default values for this component's properties
	UStateMachineComponent();
	
	/// Delegate for the enter state method
	UPROPERTY(BlueprintAssignable)
	FEnterStateSignature StateMachine_EnterState;
	
	/// Delegate for the exit state method
	UPROPERTY(BlueprintAssignable)
	FExitStateSignature StateMachine_ExitState;
	
	/// Delegate for the tick state method
	UPROPERTY(BlueprintAssignable)
	FTickStateSignature StateMachine_TickState;


protected:
	/// Called when the game starts
	virtual void BeginPlay() override;

public:	
	/// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// Function used for changing the state of the state machine. It's the only one that should be manually called
	/// @param NewState The new state for the state machine
	UFUNCTION(BlueprintCallable, Category = "State Machine")
	void SwitchState(FGameplayTag NewState);

	/// Return the initial state of the state machine
	/// @return The initial state property
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "State Machine")
	FORCEINLINE FGameplayTag GetInitialState() const { return InitialState; }

	/// Return the current state of the state machine
	/// @return The current state property
	UFUNCTION(BlueprintCallable, Category = "State Machine")
	FORCEINLINE FGameplayTag GetCurrentState() const { return CurrentState; }

	/// Return the array of accepted state
	/// @return The accepted state property
	UFUNCTION(BlueprintCallable, Category = "State Machine")
	FORCEINLINE TArray<FGameplayTag> GetAcceptedStates() const { return AcceptedStates; }
	
private:
	/// Function used for passing the changing state to the server for replication. Called by SwitchState function.
	/// @param NewState The new state for the state machine 
	UFUNCTION(Server, Reliable)
	void Server_SwitchState(FGameplayTag NewState);

	/// Function used for passing from the server to any connected clients the changing state. Called by Server_SwitchState function.
	/// @param NewState The new state for the state machine
	UFUNCTION(NetMulticast, Reliable)
	void Client_SwitchState(FGameplayTag NewState);

	/// Boolean that control the tick state event. Should be false while changing state to avoid undesired effects.
	bool bCanUseTick = false;

	/// Function triggered when a new state is entered
	void EnterState();

	/// Function triggered on tick
	/// @param DeltaTime The delta of the Component Tick.
	void TickState(float DeltaTime);

	/// Function triggered when the current state is exited
	void ExitState();

public:	

	/// Function used for debugging the state machine
	virtual void Debug();
};
