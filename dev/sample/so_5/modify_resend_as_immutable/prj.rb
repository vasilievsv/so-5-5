require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'so_5/prj.rb'
	target 'sample.so_5.modify_resend_as_immutable'

	cpp_source 'main.cpp'
}
