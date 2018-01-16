using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Xamarin.Forms;

namespace Car
{
	public class App : Application
	{
        public static float Height;
        public static float Width;
        public static string IPAddress;
		public App ()
		{
			// The root page of your application
            // The root page of your application
            MainPage = new NavigationPage(new TestCar());
		}

		protected override void OnStart ()
		{
			// Handle when your app starts
		}

		protected override void OnSleep ()
		{
			// Handle when your app sleeps
		}

		protected override void OnResume ()
		{
			// Handle when your app resumes
		}
	}
}
