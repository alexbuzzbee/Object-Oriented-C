#include <stdio.h>
#include <stdlib.h>
#include "define_EVIL/define_EVIL.h"

#define _OBJ(name) _##name##_OBJ
#define _CLASS(name) _##name##_CLASS
#define _METHTAB(name) _##name##_method_table
#define _METHS(type, interface) _##type##_##interface##_methods

struct _Interface {
	void * method_table;
	const char * name;
	struct _Interface * next;
	struct _Interface * prev;
}

struct _Object_class {
	struct _Interface * interfaces;
};

/**
 * Creates a new object with the specified interfaces.
 * @param  size       The size of the object.
 * @param  interfaces A pointer to the first interface in the interface chain.
 * @return            A pointer to the newly-created object.
 */
void * _create_object(size_t size, struct _Interface * interfaces) {
  struct _Object_class * obj = malloc(size);
  obj->interfaces = interfaces;
  return (void *) obj;
}

/**
 * Destroys the specified object.
 * @param obj The object.
 */
void _destroy_object(void *obj) {
  free(obj);
}

/**
 * Gets the specified interface to an object.
 * @param  obj  The object.
 * @param  name The name of the desired interface.
 * @return      A pointer to the interface.
 */
void * _get_interface(struct _Object_class * obj, const char *name) {
	for (struct _Interface * iface = obj->interfaces; iface != NULL; iface = iface->next) {
		if (!strcmp(iface->name, name)) {
			return iface;
		}
	}
	return NULL; // Should this be abort()?
}

void _append_interface(struct _Interface *interface, struct _Interface * to) {
	for (; to->next != NULL; to = to->next) {} // Zoom to the end of the list.
	to->next = interface;
	interface->prev = to;
}

/**
 * Defines a class.
 * @param  name       The name of the class.
 * @param  superclass The name of its superclass.
 * @param  members    A list of its members (semicolon-separated).
 * @return            Necessary definitions to make the class available.
 */
#define class(name, superclass, members) \
	typedef struct _CLASS(name) \
	{ \
		struct _CLASS(superclass); /* This is unfortunately an extension that needs enabling with -fms-extensions, but it should work on gcc, clang, and MSVC (not tested on MSVC). */ \
		EXPAND members \
	} _CLASS(name); \
	struct _Interface _CLASS(type)_interfaces = {NULL, NULL, NULL, NULL};

#define interface(name, methods) \
	struct _METHTAB(name) \
	{ \
		EXPAND methods \
	};

#define implements(type, interface) /* This part is static. */ \
	struct _METHTAB(interface) _METHS(type, interface)

#define implements_f(type, interface) /* This part has to be called inside a function. */ \
	_append_interface({_METHS(type, interface), #interface, NULL, NULL}, _CLASS(type)_interfaces)

#define new(type, name) \
  struct _CLASS(type) * _OBJ(name) = _create_object(sizeof(struct _CLASS(type)), &_CLASS(type)_interfaces)

#define delete(obj) \
  _destroy_object(_OBJ(obj)); \
  _OBJ(obj) = NULL /* Bad? */

#define callmethod(obj, type, method, ...) /* Can't figure out a way to detect class of object. */ \
	((struct _METHTAB(type) *) _OBJ(obj)->method_table)->method(IF_ELSE(IS_THING(__VA_ARGS__))(_OBJ(obj), __VA_ARGS__)(_OBJ(obj)))

#define setmethod(type, interface, name, func) /* Has to be inside a function somewhere. Require it to be at top of main() somehow? */ \
	_METHS(type, interface).name = func

// Example code below

class(Example, Object, (
  int a;
  int b;
), (
  int (*add)(struct _Example_class * self);
))

int add(struct _Example_class * self)
{
  return self->a + self->b;
}

int main(int argc, char const *argv[]) {
  setmethod(Example, add, &add);
  new(Example, exmp);
  exmp->a = 9;
  exmp->b = 4;
  printf("%i\n", callmethod(exmp, Example, add));
  delete(exmp);
  return 0;
}
