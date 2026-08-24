// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PassGameData.h"
#include "GameCameraActor.h"
#include "HPBar.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "PaperSpriteComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "Enemy.generated.h"


class UPassGameData;


UCLASS()
class PROJECT2_API AEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemy();

	//マス座標
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	int X = 0;
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	int Y = 0;

	//Payerの保存
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	TArray<AActor*> Player;

	//マス目数
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	int Width = 0;
	UPROPERTY(EditAnywhere, Category = "Character")
	int Height = 0;

	//ジャンプ音
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	USoundBase* SoundEffect;
	//攻撃音
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	USoundBase* AttackSoundEffect;
	//攻撃準備音
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	USoundBase* AttackReadySoundEffect;
	//攻撃着弾音
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	USoundBase* AttackHitSoundEffect;

	//Tileの保存
	TArray<AActor*> Tiles;

	//Magicの保存
	TArray<AActor*> Magics;

	//ストップ
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EnemyStatus")
	bool stop = true;
	
	//HP
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EnemyStatus")
	int hp = 100;

	//最大HP
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EnemyStatus")
	int maxhp = 100;

	//Enemyの思考速度
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyThinkSpeed = 1.0f;
	float ThinkTimer = 0.0f;

	//Enemyの行動速度
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyActionSpeed = 1.0f;
	float ActionTimer = 0.0f;

	//Enemyの攻撃優先度 多いほど攻撃を優先する
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyAttackPriority = 1.0f;

	//Enemyの移動目標座標
	int TargetTileX = 0;
	int TargetTileY = 0;

	//HPBarの保存
	AHPBar* HPBar;

	//Enemyの攻撃フラグ
	bool AttackFlag = true;

	//Enemyの攻撃速度
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyAttackSpeed = 1.0f;
	
	//Enemyの攻撃タイマー
	float AttackTimer = 0.0f;

	//Enemyの認識能力 多いほど正確に状況を認識する
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyRecognitionAbility = 1.0f;


	//Enemyの行動正解率　多いほど判断ミスが減る
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	float EnemyActionAccuracy = 1.0f;


	// スプライトを表示するコンポーネント（画像の見た目）
	UPROPERTY(VisibleAnywhere, Category = "EnemyStatus")
	UPaperSpriteComponent* Sprite;

	//アニメーション用スプライト
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPaperFlipbookComponent* AnimationSprite;

	// スプライトの画像（Editorで設定可能）
	UPROPERTY(EditAnywhere, Category = "EnemyStatus")
	UPaperSprite* TileSprite;

	//魔法弾の色
	UPROPERTY(EditAnywhere,Category="EnemyStatus")
	FLinearColor MagicBallColor = FLinearColor(1.0f, 0.0f, 1.0f);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Enemyの攻撃処理
	virtual void EnemyAction();

};
