#pragma once
#include "jnitl_op.h"
//
// helper classes to cache jclass/jmethodID/jfieldID
//

namespace jnitl {

// obtains the global jclass objects
class JClassID {
private:
	jclass clazz;
	const char* const name;	// class name
	
	// all JClassID instances are linked by a chain
	JClassID* next;

	static JClassID* init;

	void setup( JNIEnv* env ) {
		_ASSERT(clazz==NULL);
		clazz = static_cast<jclass>(env->NewGlobalRef(env->FindClass(name)));
		_ASSERT(clazz!=NULL);
	}

public:
	JClassID( const char* _name ) : name(_name), next(init) {
		next = init;
		init = this;
	}

	operator jclass () const {
		_ASSERT(clazz!=NULL);
		return clazz;
	}

	// called once when the system is initialized.
	static void runInit( JNIEnv* env ) {
		for( ; init!=NULL; init=init->next )
			init->setup(env);
	}
};



class JFieldID_Base {
protected:
	jfieldID id;
	JClassID& clazz;
	const char* name;
	const char* sig;

	virtual void setup( JNIEnv* env ) =0;

	// all JClassID instances are linked by a chain
	JFieldID_Base* next;

	static JFieldID_Base* init;

public:
	JFieldID_Base( JClassID& _clazz, const char* _name, const char* _sig ) : clazz(_clazz) {
		name = _name;
		sig = _sig;

		next = init;
		init = this;
	}


	// called once when the system is initialized.
	static void runInit( JNIEnv* env ) {
		for( ; init!=NULL; init=init->next )
			init->setup(env);
	}

	operator jfieldID () const {
		return id;
	}

	JClassID& getClazz() { return clazz; }
};


template < class JavaReturnType >
class JFieldID : public JFieldID_Base {
public:
	JFieldID( JClassID& _clazz, const char* _name, const char* _sig ) : JFieldID_Base(_clazz,_name,_sig) {};
protected:
	void setup( JNIEnv* env ) {
		id = env->GetFieldID( clazz, name, sig );
	}
public:
	// gets the value of this field on the given object
	JavaReturnType operator () ( JNIEnv* env, jobject o ) {
		JavaReturnType r = op::Op<JavaReturnType>::getField(env,o,id);
		return r;
	}
};


template < class JavaReturnType >
class JStaticFieldID : public JFieldID_Base {
public:
	JStaticFieldID( JClassID& _clazz, const char* _name, const char* _sig ) : JFieldID_Base(_clazz,_name,_sig) {};
protected:
	void setup( JNIEnv* env ) {
		id = env->GetStaticFieldID( clazz, name, sig );
	}
public:
	// gets the value of this field on the given object
	JavaReturnType operator () ( JNIEnv* env ) {
		JavaReturnType r = op::Op<JavaReturnType>::getStaticField(env);
		return r;
	}
};


class JMethodID_Base {
protected:
	jmethodID id;
	JClassID& clazz;
	const char* name;
	const char* sig;

	virtual void setup( JNIEnv* env ) =0;

	// all JClassID instances are linked by a chain
	JMethodID_Base* next;

	static JMethodID_Base* init;

public:
	JMethodID_Base( JClassID& _clazz, const char* _name, const char* _sig ) : clazz(_clazz) {
		name = _name;
		sig = _sig;

		next = init;
		init = this;
	}

	operator jmethodID () const {
		return id;
	}

	JClassID& getClazz() { return clazz; }

	// called once when the system is initialized.
	static void runInit( JNIEnv* env ) {
		for( ; init!=NULL; init=init->next )
			init->setup(env);
	}
};

template < class JavaReturnType >
class JMethodID : public JMethodID_Base {
public:
	JMethodID( JClassID& _clazz, const char* _name, const char* _sig ) : JMethodID_Base(_clazz,_name,_sig) {};
protected:
	void setup( JNIEnv* env ) {
		id = env->GetMethodID( clazz, name, sig );
	}
public:
	// invoke this method on a Java object
	JavaReturnType operator () ( JNIEnv* env, jobject o, ... ) {
        va_list args;
		va_start(args,o);
		JavaReturnType r = op::Op<JavaReturnType>::invokeV(env,o,id,args);
		va_end(args);
		return r;
	}
};

template < class JavaReturnType >
class JStaticMethodID : public JMethodID_Base {
public:
	JStaticMethodID( JClassID& _clazz, const char* _name, const char* _sig ) : JMethodID_Base(_clazz,_name,_sig) {};
protected:
	void setup( JNIEnv* env ) {
		id = env->GetStaticMethodID( clazz, name, sig );
	}
public:
	// invoke this method
	JavaReturnType operator () ( JNIEnv* env ... ) {
        va_list args;
		va_start(args,env);
		JavaReturnType r = op::Op<JavaReturnType>::invokeStaticV(env,clazz,id,args);
		va_end(args);
		return r;
	}
};

class JConstructorID : public JMethodID<jobject> {
public:
	JConstructorID( JClassID& _clazz, const char* _sig ) : JMethodID<jobject>(_clazz,"<init>",_sig) {};

	jobject operator () ( JNIEnv* env ... ) {
        va_list args;
		va_start(args,env);
		jobject r = env->NewObjectV(clazz,id,args);
		va_end(args);
		return r;
	}
};



extern JClassID javaLangNumber;
extern JMethodID<jbyte> javaLangNumber_byteValue;
extern JMethodID<jshort> javaLangNumber_shortValue;
extern JMethodID<jint> javaLangNumber_intValue;
extern JMethodID<jlong> javaLangNumber_longValue;
extern JMethodID<jfloat> javaLangNumber_floatValue;
extern JMethodID<jdouble> javaLangNumber_doubleValue;

extern JClassID javaLangInteger;
extern JConstructorID javaLangInteger_new;
extern JStaticMethodID<jobject> javaLangInteger_valueOf;

extern JClassID javaLangShort;
extern JStaticMethodID<jobject> javaLangShort_valueOf;

extern JClassID javaLangLong;
extern JStaticMethodID<jobject> javaLangLong_valueOf;

extern JClassID javaLangFloat;
extern JStaticMethodID<jobject> javaLangFloat_valueOf;

extern JClassID javaLangDouble;
extern JStaticMethodID<jobject> javaLangDouble_valueOf;

extern JClassID javaLangByte;
extern JStaticMethodID<jobject> javaLangByte_valueOf;

extern JClassID javaLangBoolean;
extern JMethodID<jboolean> javaLangBoolean_booleanValue;
extern JStaticMethodID<jobject> javaLangBoolean_valueOf;

extern JClassID javaLangString;

extern JClassID booleanArray;
extern JClassID byteArray;
extern JClassID charArray;
extern JClassID doubleArray;
extern JClassID floatArray;
extern JClassID intArray;
extern JClassID longArray;
extern JClassID shortArray;
extern JClassID stringArray;
extern JClassID objectArray;

}