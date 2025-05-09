#ifndef __Q3_REGISTERS__
#define __Q3_REGISTERS__

#define	MAX_VARIABLES	32

typedef std::map < std::string, std::string >		varString_m;
typedef std::map < std::string, float >		varFloat_m;

extern	varString_m	varStrings;
extern	varFloat_m	varFloats;
extern	varString_m	varVectors;

enum
{
	VTYPE_NONE = 0,
	VTYPE_FLOAT,
	VTYPE_STRING,
	VTYPE_VECTOR,
};

extern void Q3_InitVariables( void );
extern void Q3_DeclareVariable( int type, const char *name );
extern void Q3_FreeVariable( const char *name );
extern int  Q3_GetStringVariable( const char *name, const char **value );
extern int  Q3_GetFloatVariable( const char *name, float *value );
extern int  Q3_GetVectorVariable( const char *name, vec3_t value );
extern int  Q3_VariableDeclared( const char *name );
extern int  Q3_SetFloatVariable( const char *name, float value );
extern int  Q3_SetStringVariable( const char *name, const char *value );
extern int  Q3_SetVectorVariable( const char *name, const char *value );

#endif	//__Q3_REGISTERS__