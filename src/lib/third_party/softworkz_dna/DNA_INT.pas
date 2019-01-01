// Version 4.00
// April-12-2009
// Dephi 2009 

unit DNA_INT;

//***********
	interface
//***********

Const
	ERR_NO_ERROR 						 			 = 0;
  ERR_ACTIVATIONS_EXCEEDED       = -3;
  ERR_DNA_DISABLE			 					 = -2;
  ERR_VALIDATION_WARNING				 = -1;
  ERR_NO_CONNECTION 	 					 = 1;
  ERR_CONNECTION_LOST 					 = 2;
  ERR_LOCKOUT 				 					 = 3;
  ERR_INVALID_CDM 		 					 = 4;
  ERR_INVALID_PRODUCT_KEY 			 = 5;
  ERR_INVALID_ACTIVATION_CODE 	 = 6;
  ERR_INVALID_PASSWORD 					 = 7;
  ERR_ACTIVATION_EXPECTED  			 = 8;
  ERR_REACTIVATION_EXPECTED			 = 9;
  ERR_BANNED_ACTIVATION_CODE		 = 10;
  ERR_NO_EMAIL_PROVIDED					 = 11;
  ERR_INVALID_BUILD_NO 			 		 = 12;
  ERR_EVAL_CODE_ALREADY_SENT 		 = 13;
  ERR_EVAL_CODE_UNAVAILABLE 		 = 14;
  ERR_CDM_HAS_EXPIRED 		 			 = 15;
  ERR_CODE_HAS_EXPIRED 		 			 = 16;
  ERR_INVALID_NEW_PASSWORD 			 = 17;
  ERR_CDM_WRITE_PROTECTED        = 18;
  ERR_ACTIVATION_EXPECTED_MU				= 19;
  ERR_REACTIVATION_EXPECTED_MU				= 20;
  ERR_ACTIVATIONS_EXCEEDED_MU				= 21;
  ERR_CANCELLED_BY_USER				 	 = 98;
  ERR_OPERATION_FAILED					 = 99;

function DNA_Activate(product_key,activation_code,password,email:PAnsiChar):integer; stdcall;
function DNA_ActivateOffline(product_key,activation_code:PAnsiChar):integer; stdcall;
function DNA_Reactivate(product_key,activation_code,password,new_password:PAnsiChar):integer; stdcall;

function DNA_Validate(product_key:PAnsiChar):integer; stdcall;
function DNA_Validate2(product_key:PAnsiChar):integer; stdcall;
function DNA_Validate3(product_key:PAnsiChar):integer; stdcall;
function DNA_Validate4(product_key:PAnsiChar):integer; stdcall;
function DNA_Validate5(product_key:PAnsiChar):integer; stdcall;

function DNA_ValidateCDM(product_key:PAnsiChar):integer; stdcall;
function DNA_ValidateCDM2(product_key:PAnsiChar):integer; stdcall;
function DNA_ValidateCDM3(product_key:PAnsiChar):integer; stdcall;
function DNA_ValidateCDM4(product_key:PAnsiChar):integer; stdcall;
function DNA_ValidateCDM5(product_key:PAnsiChar):integer; stdcall;

function DNA_Deactivate(product_key,password:PAnsiChar):integer; stdcall;
function DNA_SendPassword(product_key,activation_code:PAnsiChar):integer; stdcall;
function DNA_Query(product_key,activation_code:PAnsiChar):integer; stdcall;
function DNA_InfoTag(product_key,activation_code,tag:PAnsiChar):integer; stdcall;
function DNA_SetBuildNo(build_no:PAnsiChar):integer; stdcall;
function DNA_SendEvalCode(product_key,email:PAnsiChar; Use_MachineID:integer):integer; stdcall;
function DNA_SetCDMPathName(path_name:PAnsiChar):integer; stdcall;
function DNA_SetINIPathName(path_name:PAnsiChar):integer; stdcall;
function DNA_ProtectionOK(product_key:PAnsiChar; Request_EvalCode,Use_MachineID:integer):integer; stdcall;
function DNA_SetProxy(server,port,username,password:PAnsiChar):integer; stdcall;
function DNA_SetLanguage(language:integer):integer; stdcall;
function DNA_EvaluateNow(product_key:PAnsiChar):integer; stdcall;
function DNA_UseIESettings(Use_IESettings:integer):integer; stdcall;

function DNA_Error(error_no:integer; msg:PAnsiChar; msg_size:integer):integer; stdcall;
function DNA_Param(param,value:PAnsiChar; value_size:integer):integer; stdcall;

//****************
	implementation
//****************

function DNA_Activate; external 'DNA.DLL';
function DNA_ActivateOffline; external 'DNA.DLL';
function DNA_Reactivate; external 'DNA.DLL';

function DNA_Validate; external 'DNA.DLL';
function DNA_Validate2; external 'DNA.DLL';
function DNA_Validate3; external 'DNA.DLL';
function DNA_Validate4; external 'DNA.DLL';
function DNA_Validate5; external 'DNA.DLL';

function DNA_ValidateCDM; external 'DNA.DLL';
function DNA_ValidateCDM2; external 'DNA.DLL';
function DNA_ValidateCDM3; external 'DNA.DLL';
function DNA_ValidateCDM4; external 'DNA.DLL';
function DNA_ValidateCDM5; external 'DNA.DLL';

function DNA_Deactivate; external 'DNA.DLL';
function DNA_SendPassword; external 'DNA.DLL';
function DNA_Query; external 'DNA.DLL';
function DNA_InfoTag; external 'DNA.DLL';
function DNA_SetBuildNo; external 'DNA.DLL';
function DNA_SendEvalCode; external 'DNA.DLL';
function DNA_SetCDMPathName; external 'DNA.DLL';
function DNA_SetINIPathName; external 'DNA.DLL';
function DNA_ProtectionOK; external 'DNA.DLL';
function DNA_SetProxy; external 'DNA.DLL';
function DNA_SetLanguage; external 'DNA.DLL';
function DNA_EvaluateNow; external 'DNA.DLL';
function DNA_UseIESettings; external 'DNA.DLL';

function DNA_Error; external 'DNA.DLL';
function DNA_Param; external 'DNA.DLL';

end.

