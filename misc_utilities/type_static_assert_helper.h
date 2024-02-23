#pragma once

namespace MiscUtilities
{
	template<typename Ttype_to_print>
	struct debug_error_type
	{
		using error_source = Ttype_to_print::garbage_val;
		//static_assert(false);
	};

	template<typename Ttype_to_print>
	void debug_error_func()
	{
		using error_source = Ttype_to_print::garbage_val;
	};

	template<typename Ttype_to_print>
	void debug_error_func(Ttype_to_print type)
	{
		using error_source = Ttype_to_print::garbage_val;
	};

};