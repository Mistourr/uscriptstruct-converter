// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectConverter.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "CustomObjectConverter.generated.h"

/**
 * 
 */
UCLASS()
class TESTJSONCONVERSION_API UCustomObjectConverter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
        UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
        static void ConvertToJson(const int32& AnyStruct) {};

        // Needed for CustomThunk
        DECLARE_FUNCTION(execConvertToJson)
        {
            // Step into the stack to get the struct
            Stack.StepCompiledIn<FStructProperty>(nullptr);
            void* StructPtr = Stack.MostRecentPropertyAddress;
            FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
            UStruct* Subclass = StructProp->Struct.Get();

            P_FINISH;

            if (StructProp && StructPtr)
            {
                FString JsonString;
                FJsonObjectConverter::UStructToJsonObjectString(Subclass, StructPtr, JsonString);
                UE_LOG(LogTemp, Log, TEXT("Struct type: %s \nJSON: %s"), *StructProp->Struct->GetName(), *JsonString);

                /* for (TFieldIterator<FProperty> It(StructProp->Struct); It; ++It)
                 {
                     FProperty* Prop = *It;
                     FString ValueString;
                     Prop->ExportTextItem(ValueString, Prop->ContainerPtrToValuePtr<void>(StructPtr), nullptr, nullptr, PPF_None);

                     UE_LOG(LogTemp, Log, TEXT("  %s = %s"), *Prop->GetName(), *ValueString);
                 }*/

            }
        }

        UFUNCTION(BlueprintCallable, meta = (CustomStructureParam = "OutStruct"))
        static void ConvertFromJson(const FString& Json, UStruct* StructTemplate, TSubclassOf<UScriptStruct> CastType, int32& OutStruct) {
            FJsonObjectConverter::JsonObjectStringToUStruct(Json, StructTemplate);
            
        };

        // ---------------------------------------------------------------------------------------------------------------------------------------------- //

        // Input and Output are both "wildcard" struct pins
        UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "InJson,OutStruct"))
        static void ConvertFromJsonThunk(const int32& InJson, int32& OutStruct);

        DECLARE_FUNCTION(execConvertFromJsonThunk)
        {
            // Step input struct
            Stack.StepCompiledIn<FStrProperty>(nullptr);
            void* InJsonPtr = Stack.MostRecentPropertyAddress;
            FStrProperty* InStructProp = CastField<FStrProperty>(Stack.MostRecentProperty);
            auto& str = InStructProp->GetPropertyValue(InJsonPtr);

            // Step output struct
            Stack.StepCompiledIn<FStructProperty>(nullptr);
            void* OutStructPtr = Stack.MostRecentPropertyAddress;
            FStructProperty* OutStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
            UStruct* Subclass = OutStructProp->Struct.Get();

            P_FINISH;

            if (OutStructProp) {
                const FString JsonString = FString::Printf(TEXT("%s"), *str);
                UE_LOG(LogTemp, Warning, TEXT("%s"), *JsonString);
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
                TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>(FJsonObject());
                FJsonSerializer::Deserialize(Reader, JsonObject);
                //FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutStructProp->Struct);
                FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), Subclass, OutStructPtr);
            }

            //if (InStructProp && OutStructProp &&
            //    InStructProp->Struct == OutStructProp->Struct) // Must be same type!
            //{
            //    // Copy struct memory
            //    InStructProp->Struct->CopyScriptStruct(OutStructPtr, InJsonPtr);

            //    UE_LOG(LogTemp, Log, TEXT("Copied struct of type: %s"), *InStructProp->Struct->GetName());
            //}
            //else
            //{
            //    UE_LOG(LogTemp, Warning, TEXT("Mismatched struct types passed to CopyStruct!"));
            //}
        }
};
