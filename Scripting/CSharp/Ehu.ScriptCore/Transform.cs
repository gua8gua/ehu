namespace Ehu
{
	public static class Transform
	{
		public static bool HasTransform => InternalCalls.HasTransform_Native();

		public static void GetPosition(out float x, out float y, out float z)
		{
			x = 0f;
			y = 0f;
			z = 0f;
			InternalCalls.GetPosition_Native(ref x, ref y, ref z);
		}

		public static void SetPosition(float x, float y, float z) =>
			InternalCalls.SetPosition_Native(x, y, z);

		public static void GetScale(out float x, out float y, out float z)
		{
			x = 0f;
			y = 0f;
			z = 0f;
			InternalCalls.GetScale_Native(ref x, ref y, ref z);
		}

		public static void SetScale(float x, float y, float z) =>
			InternalCalls.SetScale_Native(x, y, z);

		public static void GetRotationEulerDegrees(out float xDeg, out float yDeg, out float zDeg)
		{
			xDeg = 0f;
			yDeg = 0f;
			zDeg = 0f;
			InternalCalls.GetRotationEuler_Native(ref xDeg, ref yDeg, ref zDeg);
		}

		public static void SetRotationEulerDegrees(float xDeg, float yDeg, float zDeg) =>
			InternalCalls.SetRotationEuler_Native(xDeg, yDeg, zDeg);
	}
}
