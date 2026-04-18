namespace Ehu
{
	public static class Input
	{
		public static bool IsKeyPressed(int keyCode) => InternalCalls.IsKeyPressed_Native(keyCode);

		public static bool IsMouseButtonPressed(int button) => InternalCalls.IsMouseButtonPressed_Native(button);

		public static void GetMousePosition(out float x, out float y)
		{
			x = 0f;
			y = 0f;
			InternalCalls.GetMousePosition_Native(ref x, ref y);
		}
	}
}
