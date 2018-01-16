using System.Diagnostics;
using Xamarin.Forms;

namespace Car
{
    public partial class TestCar : ContentPage
    {
        public TestCar()
        {
            InitializeComponent();
        }

        private void Start_Clicked(object sender, System.EventArgs e)
        {
            App.IPAddress = IPEntry.Text;
            App.Current.MainPage.Navigation.PushAsync(new GamePage());
        }
    }
}
