using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace i2c_master
{
	public sealed partial class MainPage : Page
	{
		private MyDevice _myDevice = null;
		private DispatcherTimer _dispatchTimer = new DispatcherTimer();

		public MainPage()
		{
			this.InitializeComponent();
		}

		protected async override void OnNavigatedTo(NavigationEventArgs e)
		{
			_myDevice = new MyDevice(0x26);

			if (await _myDevice.Initialize())
			{
				_dispatchTimer.Interval = TimeSpan.FromSeconds(5);
				_dispatchTimer.Tick += DispatchTimer_Tick;
				_dispatchTimer.Start();
			}

			base.OnNavigatedTo(e);
		}

		protected override void OnNavigatedFrom(NavigationEventArgs e)
		{
			_dispatchTimer.Stop();
			_dispatchTimer.Tick -= DispatchTimer_Tick;

			if (_myDevice != null)
			{
				_myDevice.Dispose();
				_myDevice = null;
			}

			base.OnNavigatedFrom(e);
		}

		private async void DispatchTimer_Tick(object sender, object e)
		{
			try
			{
				// ***
				// *** Read the sensor
				// ***
				string json = await _myDevice.GetData();
				this.Log.Text += json + "\r\n";
			}
			catch (Exception ex)
			{
				this.Log.Text += ex.Message + "\r\n";
			}
		}
	}
}
