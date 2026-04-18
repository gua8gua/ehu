namespace Ehu
{
	public static class Time
	{
		public static float DeltaTime => InternalCalls.GetDeltaTime_Native();

		public static float FixedDeltaTime => InternalCalls.GetFixedDeltaTime_Native();
	}
}
