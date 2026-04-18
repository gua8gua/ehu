namespace Ehu
{
	public static class Entity
	{
		public static ulong CurrentEntityId => InternalCalls.GetCurrentEntityId_Native();

		public static ulong Uuid => InternalCalls.GetEntityUuid_Native();

		public static string Tag
		{
			get => InternalCalls.GetTag_Native() ?? string.Empty;
			set => InternalCalls.SetTag_Native(value ?? string.Empty);
		}
	}
}
