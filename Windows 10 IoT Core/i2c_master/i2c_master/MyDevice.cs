using System;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;

namespace i2c_master
{
	public class MyDevice : IDisposable
	{
		public enum Command
		{
			SetLock = 1,
			BlockSize = 2,
			BlockCount = 3,
			StringLength = 4,
			GetBlock = 5,
			GetLock = 6
		}

		public MyDevice(byte address, I2cBusSpeed busSpeed = I2cBusSpeed.FastMode)
		{
			this.Addrress = address;
			this.BusSpeed = busSpeed;
		}

		protected I2cBusSpeed BusSpeed { get; set; }
		protected byte Addrress { get; set; }
		protected I2cDevice Device { get; set; }

		public async Task<bool> Initialize()
		{
			bool returnValue = false;

			// ***
			// *** Get a selector string that will return all I2C controllers on the system
			// ***
			string aqs = I2cDevice.GetDeviceSelector();

			// ***
			// *** Find the I2C bus controller device with our selector string
			// ***
			var deviceInformationCollection = await DeviceInformation.FindAllAsync(aqs).AsTask();

			// ***
			// *** Make sure there is at least one...
			// ***
			if (deviceInformationCollection.Count > 0)
			{
				// ***
				// *** Set the address of the slave device (this is the
				// *** address set in the Arduino Uno sketch.
				// ***
				var settings = new I2cConnectionSettings(0x26);

				// ***
				// *** The Uno can handle fast, some micro-controllers
				// *** cannot.
				// ***
				settings.BusSpeed = I2cBusSpeed.FastMode;

				// ***
				// *** Create an I2cDevice with our selected bus controller and I2C settings
				// ***
				this.Device = await I2cDevice.FromIdAsync(deviceInformationCollection[0].Id, settings);
				returnValue = (this.Device != null);
			}

			return returnValue;
		}

		public async Task<string> GetData()
		{
			string returnValue = string.Empty;

			// ***
			// *** Just checking...
			// ***
			int isLocked = await this.GetParameter(Command.GetLock);

			try
			{
				if (await this.SetLock(true))
				{
					int blockSize = await this.GetParameter(Command.BlockSize);
					int blockCount = await this.GetParameter(Command.BlockCount);
					int stringLength = await this.GetParameter(Command.StringLength);

					StringBuilder stringBuilder = new StringBuilder();

					// ***
					// *** Read all of the blocks
					// ***
					for (byte i = 1; i <= blockCount; i++)
					{
						// ***
						// *** Get the current block.
						// ***
						string block = await this.GetBlock(i);

						// ***
						// *** Append the block to the string.
						// ***
						stringBuilder.Append(block);
					}

					// ***
					// *** Get the full JOSN string.
					// ***
					string json = stringBuilder.ToString().Replace('\0', ' ').Trim();

					// ***
					// *** Check the length.
					// ***
					if (json.Length == stringLength)
					{
						returnValue = json;
					}
					else
					{
						throw new Exception("The JSON string was not equal to the expected length.");
					}
				}
				else
				{
					// ***
					// *** Failed to lock data.
					// ***
					throw new Exception("Failed to lock data.");
				}
			}
			finally
			{
				// ***
				// *** Unlock the data so the sensor can update.
				// ***
				await this.SetLock(false);

				// ***
				// *** Just checking...
				// ***
				isLocked = await this.GetParameter(Command.GetLock);
			}

			return returnValue;
		}

		protected async Task<int> GetParameter(Command command)
		{
			// ***
			// *** Send command 1 to get block size.
			// ***
			this.Device.Write(new byte[] { (byte)command, 0 });

			// ***
			// *** Need a short delay before reading.
			// ***
			await Task.Delay(150);

			byte[] readBuffer = new byte[1];
			this.Device.Read(readBuffer);
			return (int)readBuffer[0];
		}

		protected async Task<bool> SetLock(bool locked)
		{
			// ***
			// *** Send command 4 to get a specific block.
			// ***
			this.Device.Write(new byte[] { (byte)Command.SetLock, (byte)(locked ? 1 : 0) });

			// ***
			// *** Need a short delay before reading.
			// ***
			await Task.Delay(150);

			byte[] readBuffer = new byte[1];
			this.Device.Read(readBuffer);

			// ***
			// *** A value of 1 indicates success.
			// ***
			return readBuffer[0] == 1 ? true : false;
		}

		protected async Task<string> GetBlock(byte blockIndex)
		{
			// ***
			// *** Send command 4 to get a specific block.
			// ***
			this.Device.Write(new byte[] { (byte)Command.GetBlock, blockIndex });

			// ***
			// *** Need a short delay before reading.
			// ***
			await Task.Delay(150);

			byte[] readBuffer = new byte[32];
			this.Device.Read(readBuffer);
			return ASCIIEncoding.UTF8.GetString(readBuffer);
		}

		public void Dispose()
		{
			if (this.Device != null)
			{
				this.Device.Dispose();
				this.Device = null;
			}
		}
	}
}
