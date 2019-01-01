Option Strict Off
Option Explicit On

Imports System
Imports System.IO
Imports System.Text
Imports System.Security.Cryptography

Module DNA_INT
    ' Version 3.40
    ' March 12, 2007


    Public Const ERR_NO_ERROR As Short = 0
    Public Const ERR_DNA_DISABLE As Short = -2
    Public Const ERR_VALIDATION_WARNING As Short = -1
    Public Const ERR_ACTIVATIONS_EXCEEDED As Short = -3
    Public Const ERR_NO_CONNECTION As Short = 1
    Public Const ERR_CONNECTION_LOST As Short = 2
    Public Const ERR_LOCKOUT As Short = 3
    Public Const ERR_INVALID_CDM As Short = 4
    Public Const ERR_INVALID_PRODUCT_KEY As Short = 5
    Public Const ERR_INVALID_ACTIVATION_CODE As Short = 6
    Public Const ERR_INVALID_PASSWORD As Short = 7
    Public Const ERR_ACTIVATION_EXPECTED As Short = 8
    Public Const ERR_REACTIVATION_EXPECTED As Short = 9
    Public Const ERR_BANNED_ACTIVATION_CODE As Short = 10
    Public Const ERR_NO_EMAIL_PROVIDED As Short = 11
    Public Const ERR_INVALID_BUILD_NO As Short = 12
    Public Const ERR_EVAL_CODE_ALREADY_SENT As Short = 13
    Public Const ERR_EVAL_CODE_UNAVAILABLE As Short = 14
    Public Const ERR_CDM_HAS_EXPIRED As Short = 15
    Public Const ERR_CODE_HAS_EXPIRED As Short = 16
    Public Const ERR_INVALID_NEW_PASSWORD As Short = 17
    Public Const ERR_CDM_WRITE_PROTECTED As Short = 18
    Public Const ERR_CANCELLED_BY_USER As Short = 98
    Public Const ERR_OPERATION_FAILED As Short = 99

    Declare Function DNA_Activate Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal password As String, ByVal email As String) As Short
    Declare Function DNA_ActivateOffline Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Short
    Declare Function DNA_Reactivate Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal password As String, ByVal new_password As String) As Short
    Declare Function DNA_Validate Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_Validate2 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_Validate3 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_Validate4 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_Validate5 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_ValidateCDM Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_ValidateCDM2 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_ValidateCDM3 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_ValidateCDM4 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_ValidateCDM5 Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_Deactivate Lib "DNA.DLL" (ByVal product_key As String, ByVal password As String) As Short
    Declare Function DNA_SendPassword Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Short
    Declare Function DNA_Query Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String) As Short
    Declare Function DNA_InfoTag Lib "DNA.DLL" (ByVal product_key As String, ByVal activation_code As String, ByVal tag As String) As Short
    Declare Function DNA_SetBuildNo Lib "DNA.DLL" (ByVal build_no As String) As Short
    Declare Function DNA_SendEvalCode Lib "DNA.DLL" (ByVal product_key As String, ByVal email As String, ByVal Use_MachineID As Short) As Short
    Declare Function DNA_SetCDMPathName Lib "DNA.DLL" (ByVal path_name As String) As Short
    Declare Function DNA_SetINIPathName Lib "DNA.DLL" (ByVal path_name As String) As Short
    Declare Function DNA_ProtectionOK Lib "DNA.DLL" (ByVal product_key As String, ByVal Request_EvalCode As Short, ByVal Use_MachineID As Short) As Short
    Declare Function DNA_SetProxy Lib "DNA.DLL" (ByVal server As String, ByVal port As String, ByVal username As String, ByVal password As String) As Short
    Declare Function DNA_SetLanguage Lib "DNA.DLL" (ByVal language As Short) As Short
    Declare Function DNA_EvaluateNow Lib "DNA.DLL" (ByVal product_key As String) As Short
    Declare Function DNA_UseIESettings Lib "DNA.DLL" (ByVal Use_IESettings As Short) As Short

    Private Declare Function DNA_Error_DLL Lib "DNA.DLL" Alias "DNA_Error" (ByVal error_no As Short, ByVal msg As StringBuilder, ByVal msg_size As Short) As Short
    Private Declare Function DNA_Param_DLL Lib "DNA.DLL" Alias "DNA_Param" (ByVal param As String, ByVal value As StringBuilder, ByVal value_size As Short) As Short

    Private Function NullString(ByVal s As String) As Object
        Dim i As Short
        i = InStr(s, vbNullChar)
        If i = 0 Then
            'UPGRADE_WARNING: Couldn't resolve default property of object NullString. Click for more: 'ms-help://MS.VSExpressCC.v80/dv_commoner/local/redirect.htm?keyword="6A50421D-15FE-4896-8A1B-2EC21E9037B2"'
            NullString = ""
        Else
            NullString = Left(s, i - 1)
        End If
    End Function

    Public Function DNA_Param(ByVal param As String) As String
        Dim buffer As New StringBuilder(Space(256), 256)

        If DNA_Param_DLL(param, buffer, 256) = ERR_NO_ERROR Then
            DNA_Param = buffer.ToString()
        Else
            DNA_Param = "???"
        End If
    End Function

    Public Function DNA_Error(ByVal error_no As Short) As String
        Dim buffer As New StringBuilder(Space(256), 256)
        If DNA_Error_DLL(error_no, buffer, 256) = ERR_NO_ERROR Then
            DNA_Error = buffer.ToString()
        Else
            DNA_Error = "???"
        End If
    End Function


    Public Function MD5File(ByVal DLLPathName As String) As String
        Dim MD5result As Byte()

        If (File.Exists(DLLPathName)) Then
            Dim fs As New FileStream(DLLPathName, FileMode.Open, FileAccess.Read)
            Dim md5 As New MD5CryptoServiceProvider
            MD5result = md5.ComputeHash(fs)
            fs.Close()

            ' convert hash value to hex string
            Dim sb As New StringBuilder()
            For Each outputByte As Byte In MD5result
                ' convert each byte to a Hexadecimal upper case string
                sb.Append(outputByte.ToString("x2").ToUpper())
            Next
            Return sb.ToString()
        Else
            Return ""
        End If
    End Function

End Module