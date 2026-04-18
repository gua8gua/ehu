using System.Runtime.CompilerServices;

namespace Ehu
{
	/// <summary>
	/// 与 C++ <c>ScriptGlue::RegisterInternalCalls</c> 中 <c>mono_add_internal_call</c> 名称一一对应。
	/// </summary>
	internal static class InternalCalls
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Log_Native(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float GetDeltaTime_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong GetCurrentEntityId_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetPosition_Native(ref float x, ref float y, ref float z);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetPosition_Native(float x, float y, float z);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetScale_Native(ref float x, ref float y, ref float z);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetScale_Native(float x, float y, float z);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetRotationEuler_Native(ref float xDeg, ref float yDeg, ref float zDeg);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetRotationEuler_Native(float xDeg, float yDeg, float zDeg);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool HasTransform_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong GetEntityUuid_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern string GetTag_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetTag_Native(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool IsKeyPressed_Native(int keyCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool IsMouseButtonPressed_Native(int button);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetMousePosition_Native(ref float x, ref float y);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float GetFixedDeltaTime_Native();
	}
}
