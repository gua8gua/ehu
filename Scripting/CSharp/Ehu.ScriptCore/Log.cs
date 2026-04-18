namespace Ehu
{
	public static class Log
	{
		public static void Info(string message) => InternalCalls.Log_Native(message ?? string.Empty);
	}
}
