DNA.DLL v3.4.0.995


                   software_DNA DLL Package
                         
                     Version 3.4.0 Release
                          March-28-2007

           softWORKZ Innovation Inc. All Rights Reserved.


-----------
Description
-----------
The software_DNA DLL Package contains the software_DNA Client and the Interface files required for several programming languages (including DELPHI, VBasic 6, C++ Builder,Visual C++, VB.NET, C#.NET and Sun Java). The Interface files have been fully tested and verified by softWORKZ. This version reads the proxy configuration from Windows Internet Explorer settings, therefore it does not require proxy settings to be input by the user.


-------
Install – DNA.DLL
-------
The DNA.DLL file must be packaged with your software application and MUST be installed in the same directory as your software application.


-------
Install – coding environment
-------
The following files need to be added to your software project to use the API calls from your software:

        * Borland Delphi		DNA.DLL
					DNA_INT.pas

        * Borland C++ Builder		DNA.DLL
					DNA_INT.h
					DNA_INT_BC.lib

        * Microsoft VBasic 6		DNA.DLL
					DNA_INT.bas

        * Microsoft Visual C++		DNA.DLL
					DNA_INT.h
					DNA_INT_VC.lib

                                        NOTE: During debugging you will receive several Warnings of the following format
                                        >>First-chance exception in ******.exe (*****.DLL): 0xC0000000: Access Violation.

                                        It is safe to ignore these warnings as per Microsoft:
                                        http://support.microsoft.com/support/kb/articles/q105/6/75.asp

        * Microsoft
	  Visual Studio 2005 VB.NET	DNA.DLL
					DNA_INT.vb

        * Microsoft
	  Visual Studio 2005 C#.NET	DNA.DLL
					DNA_INT.cs

        * Sun Java 			DNA.DLL
					DNAINT.java in SoftworkzDNA.zip

---------
MD5 Hash Code
---------
The MD5 Hash Code is used by your application to verify the authenticity of the DNA.DLL file. Any discrepancy would indicate a wrong version of the DNA.DLL file is being used by the user, or that the user has attempted to change or replace the DNA.DLL file in some way.
You should embed the MD5 Hash Code in your application in an encrypted form and use it to authenticate the DNA.DLL everything time you use the software_DNA API calls.

MD5 Hash Code for DNA 3.4.0 Build 995: B9AA3D17551F3712C11C5B72DF06737E


------------
Documentation and code samples
------------
Complete documentation for integrating the software_DNA protection solution into your application and code samples in several programming languages can be found in the DNA Control Panel, Documentation and Samples section, at www.softworkz.com

Available documentation includes:

	* software_DNA API Developer Guide - Do All Integration
	* software_DNA API Developer Guide - Custom Integration
	* software_DNA Sample Code Guide


-----------------
Technical support
-----------------
Report errors and problems to support@softworkz.com
Please include the following in your message:

	* Version of the DNA.DLL
	* Windows version. Also clarify whether your Windows:
		* Includes service packs and other fixes installed
		* US or international
		* OEM or not
	* Computer hardware: CPU class, memory and hard drive space available.
	* Programming language used
	* Description of your problem (provide as much information as possible)


-------
Contact
-------
Homepage: http://www.softworkz.com
Email   : support@softworkz.com


Thanks for choosing softWORKZ.