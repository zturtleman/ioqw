code

equ memset									-1
equ memcpy									-2
equ strncpy									-3
equ sin										-4
equ cos										-5
equ atan2									-6
equ sqrt									-7
equ floor									-8
equ ceil									-9
equ acos									-10

equ trap_Print								-21
equ trap_Error								-22
equ trap_Milliseconds						-23
equ trap_RealTime							-24
equ trap_SnapVector							-25
equ trap_Argc								-26
equ trap_Argv								-27
equ trap_Args								-28
equ trap_AddCommand							-29
equ trap_RemoveCommand						-30
equ trap_Cmd_ExecuteText					-31
equ trap_Cvar_Register						-32
equ trap_Cvar_Update						-33
equ trap_Cvar_Set							-34
equ trap_Cvar_SetValue						-35
equ trap_Cvar_VariableValue					-36
equ trap_Cvar_VariableIntegerValue			-37
equ trap_Cvar_VariableStringBuffer			-38
equ trap_FS_FOpenFile						-39
equ trap_FS_Read							-40
equ trap_FS_Write							-41
equ trap_FS_Seek							-42
equ trap_FS_FCloseFile						-43
equ trap_FS_GetFileList						-44
equ trap_PC_AddGlobalDefine					-45
equ trap_PC_LoadSource						-46
equ trap_PC_FreeSource						-47
equ trap_PC_ReadToken						-48
equ trap_PC_SourceFileAndLine				-49

equ trap_LocateGameData						-101
equ trap_DropClient							-102
equ trap_SendServerCommand					-103
equ trap_GetUsercmd							-104
equ trap_SetConfigstring					-105
equ trap_GetConfigstring					-106
equ trap_SetUserinfo						-107
equ trap_GetUserinfo						-108
equ trap_GetServerinfo						-109
equ trap_SetBrushModel						-110
equ trap_Trace								-111
equ trap_TraceCapsule						-112
equ trap_ClipToEntities						-113
equ trap_PointContents						-114
equ trap_InPVS								-115
equ trap_InPVSIgnorePortals					-116
equ trap_AdjustAreaPortalState				-117
equ trap_AreasConnected						-118
equ trap_LinkEntity							-119
equ trap_UnlinkEntity						-120
equ trap_EntitiesInBox						-121
equ trap_EntityContact						-122
equ trap_EntityContactCapsule				-123
equ trap_GetEntityToken						-124
equ trap_DebugPolygonCreate					-125
equ trap_DebugPolygonDelete					-126
equ trap_BotAllocateClient					-127
equ trap_BotFreeClient						-128

equ trap_BotLibSetup						-201
equ trap_BotLibShutdown						-202
equ trap_BotLibVarSet						-203
equ trap_BotLibVarGet						-204
equ trap_BotLibStartFrame					-205
equ trap_BotLibLoadMap						-206
equ trap_BotLibUpdateEntity					-207
equ trap_BotLibTest							-208
equ trap_BotGetSnapshotEntity				-209
equ trap_BotGetServerCommand				-210
equ trap_BotUserCommand						-211

equ trap_AAS_Initialized					-301
equ trap_AAS_Time							-302
equ trap_AAS_AreaInfo						-303
equ trap_AAS_EntityInfo						-304
equ trap_AAS_PresenceTypeBoundingBox		-305
equ trap_AAS_BBoxAreas						-306
equ trap_AAS_PointAreaNum					-307
equ trap_AAS_TraceAreas						-308
equ trap_AAS_PointContents					-309
equ trap_AAS_PointReachabilityAreaIndex		-310
equ trap_AAS_AreaReachability				-311
equ trap_AAS_AreaTravelTimeToGoalArea		-312
equ trap_AAS_EnableRoutingArea				-313
equ trap_AAS_PredictClientMovement			-314
equ trap_AAS_PredictRoute					-315
equ trap_AAS_AlternativeRouteGoals			-316
equ trap_AAS_ValueForBSPEpairKey			-317
equ trap_AAS_VectorForBSPEpairKey			-318
equ trap_AAS_FloatForBSPEpairKey			-319
equ trap_AAS_IntForBSPEpairKey				-320
equ trap_AAS_NextBSPEntity					-321
equ trap_AAS_Swimming						-322

equ trap_EA_GetInput						-401
equ trap_EA_ResetInput						-402
equ trap_EA_Jump							-403
equ trap_EA_DelayedJump						-404
equ trap_EA_Attack							-405
equ trap_EA_Move							-406
equ trap_EA_MoveUp							-407
equ trap_EA_MoveDown						-408
equ trap_EA_MoveForward						-409
equ trap_EA_MoveBack						-410
equ trap_EA_MoveLeft						-411
equ trap_EA_MoveRight						-412
equ trap_EA_Use								-413
equ trap_EA_View							-414
equ trap_EA_Crouch							-415
equ trap_EA_SelectWeapon					-416
equ trap_EA_Say								-417
equ trap_EA_SayTeam							-418
equ trap_EA_Command							-419
equ trap_EA_Gesture							-420
equ trap_EA_Action							-421
equ trap_EA_Talk							-422
equ trap_EA_Respawn							-423
equ trap_EA_EndRegular						-424

equ trap_BotLoadCharacter					-501
equ trap_BotFreeCharacter					-502
equ trap_Characteristic_Float				-503
equ trap_Characteristic_BFloat				-504
equ trap_Characteristic_Integer				-505
equ trap_Characteristic_BInteger			-506
equ trap_Characteristic_String				-507
equ trap_BotAllocChatState					-508
equ trap_BotFreeChatState					-509
equ trap_BotQueueConsoleMessage				-510
equ trap_BotRemoveConsoleMessage			-511
equ trap_BotNextConsoleMessage				-512
equ trap_BotNumConsoleMessages				-513
equ trap_BotNumInitialChats					-514
equ trap_BotInitialChat						-515
equ trap_BotGetChatMessage					-516
equ trap_BotReplyChat						-517
equ trap_BotChatLength						-518
equ trap_BotEnterChat						-519
equ trap_StringContains						-520
equ trap_BotFindMatch						-521
equ trap_BotMatchVariable					-522
equ trap_UnifyWhiteSpaces					-523
equ trap_BotReplaceSynonyms					-524
equ trap_BotLoadChatFile					-525
equ trap_BotSetChatGender					-526
equ trap_BotSetChatName						-527
equ trap_BotResetGoalState					-528
equ trap_BotResetAvoidGoals					-529
equ trap_BotRemoveFromAvoidGoals			-530
equ trap_BotPushGoal						-531
equ trap_BotPopGoal							-532
equ trap_BotEmptyGoalStack					-533
equ trap_BotDumpAvoidGoals					-534
equ trap_BotDumpGoalStack					-535
equ trap_BotGoalName						-536
equ trap_BotGetTopGoal						-537
equ trap_BotGetSecondGoal					-538
equ trap_BotChooseLTGItem					-539
equ trap_BotChooseNBGItem					-540
equ trap_BotTouchingGoal					-541
equ trap_BotItemGoalInVisButNotVisible		-542
equ trap_BotGetLevelItemGoal				-543
equ trap_BotSetAvoidGoalTime				-544
equ trap_BotAvoidGoalTime					-545
equ trap_BotInitLevelItems					-546
equ trap_BotUpdateEntityItems				-547
equ trap_BotLoadItemWeights					-548
equ trap_BotFreeItemWeights					-549
equ trap_BotSaveGoalFuzzyLogic				-550
equ trap_GeneticParentsAndChildSelection	-551
equ trap_BotInterbreedGoalFuzzyLogic		-552
equ trap_BotMutateGoalFuzzyLogic			-553
equ trap_BotGetNextCampSpotGoal				-554
equ trap_BotGetMapLocationGoal				-555
equ trap_BotPredictVisiblePosition			-556
equ trap_BotAllocGoalState					-557
equ trap_BotFreeGoalState					-558
equ trap_BotResetMoveState					-559
equ trap_BotMoveToGoal						-560
equ trap_BotMoveInDirection					-561
equ trap_BotResetAvoidReach					-562
equ trap_BotResetLastAvoidReach				-563
equ trap_BotAddAvoidSpot					-564
equ trap_BotReachabilityArea				-565
equ trap_BotMovementViewTarget				-566
equ trap_BotAllocMoveState					-567
equ trap_BotFreeMoveState					-568
equ trap_BotInitMoveState					-569
equ trap_BotChooseBestFightWeapon			-570
equ trap_BotGetWeaponInfo					-571
equ trap_BotLoadWeaponWeights				-572
equ trap_BotAllocWeaponState				-573
equ trap_BotFreeWeaponState					-574
equ trap_BotResetWeaponState				-575
