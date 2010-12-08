/*-----------------------------------------------------------------------------
 auto_ptr.hpp
------------------------------------------------------------------------------*/
#ifndef _AUTO_PTR_HPP_
#define _AUTO_PTR_HPP_
template <class T>
class TAutoPtr {
public:
	TAutoPtr(): data(0) {}
	TAutoPtr(T *a): data(a) {}
	inline T& operator * () {
		return *data;
	}
	inline const T& operator * () const {
		return *data;
	}
	inline T& operator [] (int i) {
		return data[i];
	}
	inline const T& operator [] (int i) const {
		return data[i];
	}
	inline TAutoPtr<T>& operator = (T* a) {
		data = a;
		return *this;
	}
	inline operator T* () {
		return data;
	}
	inline operator const T* () {
		return data;
	}
	~TAutoPtr() {
		if(data)
			delete data;
	};
private:
	T *data;
};
#endif
