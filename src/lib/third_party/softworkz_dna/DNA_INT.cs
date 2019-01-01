// software_DNA C# Interface
// Version 3.4.0 
// March 12 2007

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.IO;



namespace softworkz
{
    public static class DNA
    {
        public const int ERR_NO_ERROR                   = 0;
        public const int ERR_DNA_DISABLE		        =-2;
        public const int ERR_ACTIVATIONS_EXCEEDED       = -3;
        public const int ERR_VALIDATION_WARNING         =-1;
        public const int ERR_NO_CONNECTION 	 		    = 1;
        public const int ERR_CONNECTION_LOST 			= 2;
        public const int ERR_LOCKOUT 					= 3;
        public const int ERR_INVALID_CDM 		 		= 4;
        public const int ERR_INVALID_PRODUCT_KEY 		= 5;
        public const int ERR_INVALID_ACTIVATION_CODE	= 6;
        public const int ERR_INVALID_PASSWORD 			= 7;
        public const int ERR_ACTIVATION_EXPECTED  		= 8;
        public const int ERR_REACTIVATION_EXPECTED		= 9;
        public const int ERR_BANNED_ACTIVATION_CODE	    =10;
        public const int ERR_NO_EMAIL_PROVIDED			=11;
        public const int ERR_INVALID_BUILD_NO 			=12;
        public const int ERR_EVAL_CODE_ALREADY_SENT 	=13;
        public const int ERR_EVAL_CODE_UNAVAILABLE 	    =14;
        public const int ERR_CDM_HAS_EXPIRED 		 	=15;
        public const int ERR_CODE_HAS_EXPIRED 		 	=16;
        public const int ERR_INVALID_NEW_PASSWORD               =17;
        public const int ERR_CDM_WRITE_PROTECTED        =18;
        public const int ERR_CANCELLED_BY_USER			=98;
        public const int ERR_OPERATION_FAILED           =99;


        [DllImport("DNA.dll")]
        public static extern int DNA_ProtectionOK(string product_key, int Request_EvalCode, int Use_MachineID);
        [DllImport("DNA.dll")]
        private static extern int DNA_Error(int error_no, StringBuilder msg, ref int msg_size);
        [DllImport("DNA.dll")]
        public static extern int DNA_Activate(string product_key,string activation_code,string password,string email);
        [DllImport("DNA.dll")]
        public static extern int DNA_ActivateOffline(string product_key, string activation_code);
        [DllImport("DNA.dll")]
        public static extern int DNA_Reactivate(string product_key, string activation_code, string password, string new_password);
        [DllImport("DNA.dll")]
        public static extern int DNA_Validate(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_Validate2(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_Validate3(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_Validate4(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_Validate5(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_ValidateCDM(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_ValidateCDM2(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_ValidateCDM3(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_ValidateCDM4(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_ValidateCDM5(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_Deactivate(string product_key, string password);
        [DllImport("DNA.dll")]
        public static extern int DNA_SendPassword(string product_key, string activation_code);
        [DllImport("DNA.dll")]
        public static extern int DNA_Query(string product_key, string activation_code);
        [DllImport("DNA.dll")]
        public static extern int DNA_InfoTag(string product_key, string activation_code, string tag);
        [DllImport("DNA.dll")]
        public static extern int DNA_SetBuildNo(string build_no);
        [DllImport("DNA.dll")]
        public static extern int DNA_SendEvalCode(string product_key, string email, int Use_MachineID);
        [DllImport("DNA.dll")]
        public static extern int DNA_SetCDMPathName(string path_name);
        [DllImport("DNA.dll")]
        public static extern int DNA_SetINIPathName(string path_name);
        [DllImport("DNA.dll")]
        public static extern int DNA_SetProxy(string server, string port, string username, string password);
        [DllImport("DNA.dll")]
        public static extern int DNA_SetLanguage(int language);
        [DllImport("DNA.dll")]
        public static extern int DNA_EvaluateNow(string product_key);
        [DllImport("DNA.dll")]
        public static extern int DNA_UseIESettings(int Use_IESettings);

        [DllImport("DNA.dll")]
        private static extern int DNA_Param(string param, StringBuilder value, ref int value_size);

        public static string DNA_Error(int err)
        {
            StringBuilder ErrorMsg = new StringBuilder(256);
            int msgSize = ErrorMsg.Capacity;

            if (DNA_Error(err, ErrorMsg, ref msgSize) == ERR_NO_ERROR)
            {
                return ErrorMsg.ToString();
            }
            else
            {
                return "???";
            }
        }

        public static string DNA_Param(string param)
        {
            StringBuilder value = new StringBuilder(256);
            int valueSize = value.Capacity;

            if (DNA_Param(param, value, ref valueSize) == ERR_NO_ERROR)
            {
                return value.ToString();
            }
            else
            {
                return "???";
            }
        }

        public static string MD5File(string DLLPathName)
        {
            byte[] MD5result;

            if (File.Exists(DLLPathName))
            {
                FileStream fs = new FileStream(DLLPathName, FileMode.Open, FileAccess.Read);
                MD5 md5 = new MD5CryptoServiceProvider();
                MD5result = md5.ComputeHash(fs);
                fs.Close();

                // convert hash value to hex string
                StringBuilder sb = new StringBuilder();
                foreach (byte outputByte in MD5result)
                {
                    // convert each byte to a Hexadecimal upper case string
                    sb.Append(outputByte.ToString("x2").ToUpper());
                }

                return sb.ToString();
            }
            else
            {
                return "";
            }

        }


    }
}
