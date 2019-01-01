// Version 4.00D
// November-17-2009

#define ERR_NO_ERROR					0
#define ERR_ACTIVATIONS_EXCEEDED		-3
#define ERR_DNA_DISABLE			 		-2
#define ERR_VALIDATION_WARNING			-1
#define ERR_NO_CONNECTION 	 			1
#define ERR_CONNECTION_LOST 			2
#define ERR_LOCKOUT 				 	3
#define ERR_INVALID_CDM 		 		4
#define ERR_INVALID_PRODUCT_KEY 		5
#define ERR_INVALID_ACTIVATION_CODE		6
#define ERR_INVALID_PASSWORD 			7
#define ERR_ACTIVATION_EXPECTED  		8
#define ERR_REACTIVATION_EXPECTED		9
#define ERR_BANNED_ACTIVATION_CODE		10
#define ERR_NO_EMAIL_PROVIDED			11
#define ERR_INVALID_BUILD_NO 			12
#define ERR_EVAL_CODE_ALREADY_SENT 		13
#define ERR_EVAL_CODE_UNAVAILABLE 		14
#define ERR_CDM_HAS_EXPIRED 		 	15
#define ERR_CODE_HAS_EXPIRED 		 	16
#define ERR_INVALID_NEW_PASSWORD 		17
#define ERR_CDM_WRITE_PROTECTED			18
#define ERR_ACTIVATION_EXPECTED_MU		19
#define ERR_REACTIVATION_EXPECTED_MU		20
#define ERR_ACTIVATIONS_EXCEEDED_MU		21
#define ERR_CANCELLED_BY_USER			98
#define ERR_OPERATION_FAILED			99

extern "C" {
	int __stdcall DNA_Activate(char *product_key,char *activation_code,char *password,char *email);
	int __stdcall DNA_ActivateOffline(char *product_key,char *activation_code);
	int __stdcall DNA_Reactivate(char *product_key,char *activation_code,char *password,char *new_password);

	int __stdcall DNA_Validate(char *product_key);
	int __stdcall DNA_Validate2(char *product_key);
	int __stdcall DNA_Validate3(char *product_key);
	int __stdcall DNA_Validate4(char *product_key);
	int __stdcall DNA_Validate5(char *product_key);

	int __stdcall DNA_ValidateCDM(char *product_key);
	int __stdcall DNA_ValidateCDM2(char *product_key);
	int __stdcall DNA_ValidateCDM3(char *product_key);
	int __stdcall DNA_ValidateCDM4(char *product_key);
	int __stdcall DNA_ValidateCDM5(char *product_key);

	int __stdcall DNA_Deactivate(char *product_key,char *password);
	int __stdcall DNA_SendPassword(char *product_key,char *activation_code);
	int __stdcall DNA_Query(char *product_key,char *activation_code);
	int __stdcall DNA_InfoTag(char *product_key,char *activation_code,char *tag);
	int __stdcall DNA_SetBuildNo(char *build_no);
	int __stdcall DNA_SendEvalCode(char *product_key,char *email,int Use_MachineID);
	int __stdcall DNA_SetCDMPathName(char *path_name);
	int __stdcall DNA_SetINIPathName(char *path_name);
	int __stdcall DNA_ProtectionOK(char *product_key,int Request_EvalCode,int Use_MachineID);
	int __stdcall DNA_SetProxy(char *server,char *port,char *username,char *password);
	int __stdcall DNA_SetLanguage(int language);
	int __stdcall DNA_EvaluateNow(char *product_key);
	int __stdcall DNA_UseIESettings(int Use_IESettings);

	int __stdcall DNA_Error(int error_no,char *msg,int msg_size);
	int __stdcall DNA_Param(char *param,char *value,int value_size);
}
