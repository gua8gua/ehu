using Ehu;

namespace Game
{
	/// <summary>
	/// 占位类型：确保 Game 程序集非空；用户脚本可与本类并存。
	/// 在实体上追加一个脚本实例并设置 <c>ScriptComponent.Instances[n].ClassName = "Game.PlaceholderScript"</c> 即可绑定。
	/// </summary>
	public class PlaceholderScript
	{
		public void OnCreate()
		{
			Log.Info("PlaceholderScript OnCreate");
		}

		public void OnUpdate(float dt)
		{
			_ = dt;
		}
	}
}
