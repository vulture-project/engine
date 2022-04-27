#pragma once

namespace ecs {
	namespace utility {

		template<class T>
		class TypeIdGenerator {
		public:
			TypeIdGenerator();
			~TypeIdGenerator();

			using TypeId = size_t;

			template<class U>
			static const TypeId& Get();

			static const TypeId& Get();

		private:
			static TypeId type_counter;
		};

		template <typename T>
		typename TypeIdGenerator<T>::TypeId TypeIdGenerator<T>::type_counter = 0;
		

		template<class T>
		inline TypeIdGenerator<T>::TypeIdGenerator() {

		}

		template<class T>
		inline TypeIdGenerator<T>::~TypeIdGenerator() {

		}

		template<class T>
		template<class U>
		inline const typename TypeIdGenerator<T>::TypeId& TypeIdGenerator<T>::Get() {
			static const TypeIdGenerator::TypeId genereted_type_id = type_counter++;
			return genereted_type_id;
		}

		
		template<class T>
		inline const typename TypeIdGenerator<T>::TypeId& TypeIdGenerator<T>::Get() {
			return type_counter;
		}
		

	} //namespace utility
} //namespace ecs