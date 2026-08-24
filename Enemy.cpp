// Fill out your copyri
// ght notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Tile.h"
#include "HPBar.h"
#include "MagicBall.h"
#include "PlayerCharacter.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AEnemy::AEnemy(){
	PrimaryActorTick.bCanEverTick = true;
	//スプライトコンポーネントの作成
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	AnimationSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("AnimationSprite"));
	RootComponent = AnimationSprite;

	//このアクターを動けるようにする。
	AnimationSprite->SetMobility(EComponentMobility::Movable);
	//スプライトのレイヤー設定
	Sprite->SetTranslucentSortPriority(5);

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay(){
	Super::BeginPlay();

	//マス目の取得
	UPassGameData* GameData = Cast<UPassGameData>(GetGameInstance());
	if (GameData)
	{
		Width = GameData->SelectedStageData.GridWidth;
		Height = GameData->SelectedStageData.GridHeight;
	}

	//中央にセット
	X = Width / 2;
	Y = Height / 2;

	//フィールド上のタイルの取得
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

	for (AActor* Tile : Tiles)
	{
		ATile* UseTile = Cast <ATile>(Tile);
		if (UseTile->x == X && UseTile->y == Y && UseTile->side == false) {
			FVector TileLocation = UseTile->GetActorLocation();
			//見やすいように少し上にずらす
			TileLocation.Y = TileLocation.Y + 10;
			SetActorLocation(TileLocation);

		}
	}
	//スプライトのレイヤー設定
	Sprite->SetTranslucentSortPriority(5);

	//各種バーの取得
	FVector BarLocation = GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	HPBar = GetWorld()->SpawnActor<AHPBar>(AHPBar::StaticClass(), FVector(0.0f, 1.0f, 0.0f), FRotator::ZeroRotator);
	HPBar->MaxHp = maxhp;
	HPBar->barColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
	HPBar->SetActorLocation(BarLocation);

	TargetTileX = X;
	TargetTileY = Y;

	//作り終わったら適宜消すこと
	//stop = false;

}

// Called every frame
void AEnemy::Tick(float DeltaTime){
	Super::Tick(DeltaTime);
	float random;
	//フィールド上のタイルの取得
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);
	
	//HPの制限処理
	if (hp < 0) {
		hp = 0;
	}
	else if(hp > maxhp){
		hp = maxhp;
	}
	if (hp <= 0) {
		stop = true;
	}

	//位置修正処理
	for (AActor* Tile : Tiles)
	{
		ATile* UseTile = Cast <ATile>(Tile);
		if (UseTile->x == X && UseTile->y == Y && UseTile->side == false) {
			FVector TileLocation = UseTile->GetActorLocation();

			TileLocation.Z = TileLocation.Z +120;
			SetActorLocation(TileLocation);

		}
	}

	//HPバーの位置更新
	FVector BarLocation = GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	HPBar->Sprite->SetTranslucentSortPriority(9);
	HPBar->MaxHp = maxhp;
	HPBar->SetActorLocation(BarLocation);
	HPBar->hp = hp;

	//Enemyの思考処理
	
	//タイマー処理
	ThinkTimer += DeltaTime;
	ActionTimer += DeltaTime;

	//攻撃処理(攻撃可能なら)
	if (AttackFlag == true) {
		AttackTimer += DeltaTime;
		if (AttackTimer >= EnemyAttackSpeed && stop == false) {

			UGameplayStatics::PlaySound2D(this, AttackSoundEffect);
			AttackTimer = 0;
			for (AActor* MagicBall : Magics)
			{
				if (!IsValid(MagicBall))
					continue;

				AMagicBall* UseMagicBall = Cast<AMagicBall>(MagicBall);
				if (!UseMagicBall)
					continue;

				if (UseMagicBall->Stey && UseMagicBall->TargetTile)
				{
					UseMagicBall->Stey = false;
				}
			}

		}
	}
	else {
		AttackTimer = 0;
	}

	if (ThinkTimer >= EnemyThinkSpeed && stop == false) {
		//思考処理
		ThinkTimer = 0.0f;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagicBall::StaticClass(), Magics);
		
		//そのままで被弾するかの判定
		bool Hitflag = false;
		for (AActor* MagicBall : Magics)
		{
			AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
			if (UseMagicBall->TargetX == X && UseMagicBall->TargetY == Y && UseMagicBall->TargetTile == false) {
				random = FMath::FRand(); // 0.0?1.0 のランダム
				//認識判定
				//if (random >= EnemyRecognitionAbility) {
				//}
				//else {
				//	Hitflag = true;
				//}
				Hitflag = true;
				break;
			}
		}

		//攻撃優先の場合回避しない
		if (AttackFlag == true) {
			float Progress = AttackTimer / EnemyAttackSpeed;
			if(Progress < EnemyAttackPriority) {
				Hitflag = false;
			}
		}


		//回避の判定ミス
		random = FMath::FRand(); // 0.0?1.0 のランダム
		if (random >= EnemyActionAccuracy) {
			Hitflag = false;
		}


		//最短の安全経路算出
		if (Hitflag == true) {
			TargetTileX = X;
			TargetTileY = Y;

			float ManhattanA = 1000;
			float ManhattanB = 0;

			bool outflag = false;

			random = FMath::FRand();
			if (random >= 0.5f) {
				for (int i = Width - 1; i >= 0; i--) {
					for (int j = Height - 1; j >= 0; j--) {
						outflag = false;
						for (AActor* MagicBall : Magics)
						{
							AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
							if (UseMagicBall->TargetX == i && UseMagicBall->TargetY == j && UseMagicBall->TargetTile == false) {
								//認識判定
								random = FMath::FRand(); // 0.0?1.0 のランダム
								if (random >= EnemyRecognitionAbility) {
								}
								else {
									outflag = true;
								}
								
								break;
							}
						}
						if (outflag == false) {
							ManhattanB = FMath::Abs(i - X) + FMath::Abs(j - Y);
							if (ManhattanA > ManhattanB) {
								ManhattanA = ManhattanB;
								TargetTileX = i;
								TargetTileY = j;
							}
						}
					}
				}
			}
			else {
				for (int i = 0; i < Width; i++) {
					for (int j = 0; j < Height; j++) {
						outflag = false;
						for (AActor* MagicBall : Magics)
						{
							AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
							if (UseMagicBall->TargetX == i && UseMagicBall->TargetY == j && UseMagicBall->TargetTile == false) {
								outflag = true;
								break;
							}
						}
						if (outflag == false) {
							ManhattanB = FMath::Abs(i - X) + FMath::Abs(j - Y);
							if (ManhattanA > ManhattanB) {
								ManhattanA = ManhattanB;
								TargetTileX = i;
								TargetTileY = j;
							}
						}
					}
				}
			}
		}


	}

	if (ActionTimer >= EnemyActionSpeed && stop == false) {
		//行動処理
		ActionTimer = 0.0f;

		if (TargetTileX == X && TargetTileY == Y) {
			//攻撃処理
			random = FMath::FRand(); // 0.0?1.0 のランダム
			if (random >= EnemyActionAccuracy) {

			}
			else {
				EnemyAction();
				AttackFlag = true;
			}
		}
		else {
			//移動処理
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagicBall::StaticClass(), Magics);
			UGameplayStatics::PlaySound2D(this, SoundEffect);

			//移動するため魔法弾の削除
			for (AActor* MagicBall : Magics)
			{
				AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
				if (UseMagicBall->Stey == true && UseMagicBall->TargetTile == true) {
					UseMagicBall->Destroy();
				}
			}
			AttackFlag = false;
			if (X < TargetTileX) {
				X++;
			}
			else if (X > TargetTileX) {
				X--;
			}
			else if (Y < TargetTileY) {
				Y++;
			}
			else if (Y > TargetTileY) {
				Y--;
			}
		}
	}
}

void AEnemy::EnemyAction()
{
	//Enemyの行動処理

	FRotator SpawnRotation = FRotator::ZeroRotator;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), Player);

	//円の半径	
	float Radius = 300.0f;

	UGameplayStatics::PlaySound2D(this, AttackReadySoundEffect);
	for (AActor* PlayerCharacter : Player)
	{
		APlayerCharacter* UsePlayer = Cast <APlayerCharacter>(PlayerCharacter);

		for (int i = 0; i < Width; i++) {

			for (AActor* Tile : Tiles)
			{
				ATile* UseTile = Cast <ATile>(Tile);
				if (UseTile->x == i && UseTile->y == UsePlayer->Y && UseTile->side == true) {

					FVector CharacterLocation = GetActorLocation();
					float Angle = (2 * PI / Width) * i;
					CharacterLocation.X = CharacterLocation.X + Radius * FMath::Cos(Angle);
					CharacterLocation.Z = CharacterLocation.Z + Radius * FMath::Sin(Angle);

					AMagicBall* NewMagicBall = GetWorld()->SpawnActor<AMagicBall>(AMagicBall::StaticClass(), CharacterLocation, SpawnRotation);
					NewMagicBall->AnimationSprite->SetSpriteColor(FLinearColor(0.8f, 0.1f, 0.8f));
					NewMagicBall->Target = UseTile->GetActorLocation();
					NewMagicBall->TargetX = i;
					NewMagicBall->TargetY = UsePlayer->Y;
					NewMagicBall->AttackPower = 10;
					NewMagicBall->TargetTile = true;
					NewMagicBall->AttackSoundEffect = AttackHitSoundEffect;
					FVector MyLocation = GetActorLocation();
					FVector TargetLocation = UseTile->GetActorLocation();
					MyLocation.Y = 0;
					TargetLocation.Y = 0;
					FRotator NewRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);
					NewRot.Pitch += 90.0f;
					NewMagicBall->SetActorRotation(NewRot);
				}

			}
		}
	}
}
