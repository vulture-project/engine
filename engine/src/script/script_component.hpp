#include <sol/sol.hpp>

class CScriptProxy {
public:
	CScriptProxy() = default;
	CScriptProxy(const std::string& script_file);

	template <typename ComponentName, typename... Args>
	void NewUsertype(Args&&... args) {
		(*lua_script).new_usertype<ComponentName>(std::forward<Args>(args)...);
	}

	template <typename T>
	void SetInstance(const char* name_in_lua, T obj) {
		(*lua_script)[name_in_lua] = obj;
	}

	void Run();
private:
	sol::state* lua_script;
	sol::load_result* load_result;
};