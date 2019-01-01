' Version 3.40
' November-12-2006

Attribute VB_Name = "DNA_INT"

Public Const ERR_NO_ERROR = 0
Public Const ERR_ACTIVATIONS_EXCEEDED = -3
Public Const ERR_DNA_DISABLE = -2
Public Const ERR_VALIDATION_WARNING = -1
Public Const ERR_NO_CONNECTION = 1
Public Const ERR_CONNECTION_LOST = 2
Public Const ERR_LOCKOUT = 3
Public Const ERR_INVALID_CDM = 4
Public Const ERR_INVALID_PRODUCT_KEY = 5
Public Const ERR_INVALID_ACTIVATION_CODE = 6
Public Const ERR_INVALID_PASSWORD = 7
Public Const ERR_ACTIVATION_EXPECTED = 8
Public Const ERR_REACTIVATION_EXPECTED = 9
Public Const ERR_BANNED_ACTIVATION_CODE = 10
Public Const ERR_NO_EMAIL_PROVIDED = 11
Public Const ERR_INVALID_BUILD_NO = 12
Public Const ERR_EVAL_CODE_ALREADY_SENT = 13
Public Const ERR_EVAL_CODE_UNAVAILABLE = 14
Public Const ERR_CDM_HAS_EXPIRED = 15
Public Const ERR_CODE_HAS_EXPIRED = 16
Public Const ERR_INVALID_NEW_PASSWORD = 17
Public Const ERR_CDM_WRITE_PROTECTED = 18
Public Const ERR_CANCELLED_BY_USER = 98
Public Const ERR_OPERATION_FAILED = 99

Declare Function DNA_Activate Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal password As String, ByVal email As String) As Integer
Declare Function DNA_ActivateOffline Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Integer
Declare Function DNA_Reactivate Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal password As String, ByVal new_password As String) As Integer

Declare Function DNA_Validate Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_Validate2 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_Validate3 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_Validate4 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_Validate5 Lib "DNA.DLL" (ByVal product_key As String) As Integer

Declare Function DNA_ValidateCDM Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_ValidateCDM2 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_ValidateCDM3 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_ValidateCDM4 Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_ValidateCDM5 Lib "DNA.DLL" (ByVal product_key As String) As Integer

Declare Function DNA_Deactivate Lib "DNA.DLL" (ByVal product_key As String, ByVal password As String) As Integer
Declare Function DNA_SendPassword Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Integer
Declare Function DNA_Query Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Integer
Declare Function DNA_InfoTag Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal tag As String) As Integer
Declare Function DNA_SetBuildNo Lib "DNA.DLL" (ByVal build_no As String) As Integer
Declare Function DNA_SendEvalCode Lib "DNA.DLL" (ByVal product_key As String, ByVal email As String, ByVal Use_MachineID As Integer) As Integer
Declare Function DNA_SetCDMPathName Lib "DNA.DLL" (ByVal path_name As String) As Integer
Declare Function DNA_SetINIPathName Lib "DNA.DLL" (ByVal path_name As String) As Integer
Declare Function DNA_ProtectionOK Lib "DNA.DLL" (ByVal product_key As String, ByVal Request_EvalCode As Integer, ByVal Use_MachineID As Integer) As Integer
Declare Function DNA_SetProxy Lib "DNA.DLL" (ByVal server As String, ByVal port As String, ByVal username As String, ByVal password As String) As Integer
Declare Function DNA_SetLanguage Lib "DNA.DLL" (ByVal language As Integer) As Integer
Declare Function DNA_EvaluateNow Lib "DNA.DLL" (ByVal product_key As String) As Integer
Declare Function DNA_UseIESettings Lib "DNA.DLL" (ByVal Use_IESettings As Integer) As Integer

Private Declare Function DNA_Error_DLL Lib "DNA.DLL" Alias "DNA_Error" (ByVal error_no As Integer, ByVal msg As String, ByVal msg_size As Integer) As Integer
Private Declare Function DNA_Param_DLL Lib "DNA.DLL" Alias "DNA_Param" (ByVal param As String, ByVal value As String, ByVal value_size As Integer) As Integer

Private Function NullString(ByVal s As String)
Dim i As Integer
    i = InStr(s, vbNullChar)
    If i = 0 Then
        NullString = ""
    Else
        NullString = Left(s, i - 1)
    End If
End Function

Public Function DNA_Param(ByVal param As String, ByRef value As String, ByVal value_size As Integer) As Integer
Dim buffer As String

    buffer = Space(value_size)
    DNA_Param = DNA_Param_DLL(param, buffer, value_size)
    value = NullString(buffer)
End Function

Public Function DNA_Error(ByVal error_no As Integer, ByRef msg As String, ByVal msg_size As Integer) As Integer
Dim buffer As String

    buffer = Space(msg_size)
    DNA_Error = DNA_Error_DLL(error_no, buffer, msg_size)
    msg = NullString(buffer)
End Function
