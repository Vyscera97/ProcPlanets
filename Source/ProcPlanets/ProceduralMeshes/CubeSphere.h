// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "CubeSphere.generated.h"

UCLASS()
class PROCPLANETS_API ACubeSphere : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Mesh)
	int32 Radius;

	UPROPERTY(EditAnywhere, Category = Mesh)
	int32 VertexDistance;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	int32 Resolution;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector> Vertices;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<int32> Triangles;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector> Normals;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector2D> UV0;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FLinearColor> VertexColors;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FProcMeshTangent> Tangents;

public:
	// Sets default values for this actor's properties
	ACubeSphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* Mesh;

	void GenerateMesh();
	void CreateVertices();
	void CreateTriangles();


//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
//#endif

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//virtual void OnConstruction(const FTransform& Transform) override;

private:
	//FCriticalSection MeshGenCriticalSection;

};
