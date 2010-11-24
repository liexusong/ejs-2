/**
    ejsHelper.c - Helper methods for types.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static MprNumber parseNumber(Ejs *ejs, MprChar *str);
static bool      parseBoolean(Ejs *ejs, MprChar *s);

/************************************* Code ***********************************/

EjsAny *ejsAlloc(Ejs *ejs, EjsType *type, int extra)
{
    EjsObj      *vp;

    mprAssert(type);
    mprAssert(extra >= 0);

    if ((vp = mprAllocBlock(ejs, type->instanceSize + extra, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == NULL) {
        return NULL;
    }
    vp->type = type;
    mprAssert(type->manager);
    mprSetManager(vp, type->manager);
#if UNUSED
    //  MOB - OPT
    if (MPR->heap.requireGC) {
        ejsAttention(ejs);
    }
#endif
    return vp;
}


/**
    Cast the variable to a given target type.
    @return Returns a variable with the result of the cast or null if an exception is thrown.
 */
EjsAny *ejsCast(Ejs *ejs, EjsAny *vp, EjsType *targetType)
{
    EjsType     *type;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(targetType);

    type = TYPE(vp);
    if (type == targetType) {
        return vp;
    }
    if (type->helpers.cast) {
        return (type->helpers.cast)(ejs, vp, targetType);
    }
    ejsThrowInternalError(ejs, "Cast helper not defined for type \"%@\"", type->qname.name);
    return 0;
}


/*
    Create a new instance of a variable. Delegate to the type specific create.
 */
EjsAny *ejsCreate(Ejs *ejs, EjsType *type, int numSlots)
{
#if VXWORKS
    /*
     *  The VxWorks cc386 invoked linker crashes without this test. Ugh!
     */
    if (type == 0) {
        return 0;
    }
#endif
    mprAssert(type->helpers.create);
    return (type->helpers.create)(ejs, type, numSlots);
}


/**
    Copy a variable by copying all properties. If a property is a reference  type, just copy the reference.
    See ejsDeepClone for a complete recursive copy of all reference contents.
    @return Returns a variable or null if an exception is thrown.
 */
EjsAny *ejsClone(Ejs *ejs, EjsAny *src, bool deep)
{
    EjsAny  *dest;
    
    if (src == 0) {
        return 0;
    }
    mprAssert(TYPE(src)->helpers.clone);
    if (VISITED(src) == 0) {
        VISITED(src) = 1;
        dest = (TYPE(src)->helpers.clone)(ejs, src, deep);
        BUILTIN(dest) = BUILTIN(src);
        DYNAMIC(dest) = DYNAMIC(src);
        VISITED(src) = 0;
    } else {
        dest = src;
    }
    return dest;
}


/*
    Define a property and its traits.
    @return Return the slot number allocated for the property.
 */
int ejsDefineProperty(Ejs *ejs, EjsAny *vp, int slotNum, EjsName name, EjsType *propType, int64 attributes, EjsAny *value)
{
    mprAssert(name.name);
    mprAssert(name.space);
    
    return (TYPE(vp)->helpers.defineProperty)(ejs, vp, slotNum, name, propType, attributes, value);
}


/**
    Delete a property in a value
    @return Returns a status code.
 */
int ejsDeleteProperty(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsType     *type;

    mprAssert(slotNum >= 0);
    
    type = TYPE(vp);
    mprAssert(type->helpers.deleteProperty);
    return (type->helpers.deleteProperty)(ejs, vp, slotNum);
}


/**
    Delete a property
    @return Returns a status code.
 */
int ejsDeletePropertyByName(Ejs *ejs, EjsAny *vp, EjsName qname)
{
    EjsLookup   lookup;
    int         slotNum;

    mprAssert(qname.name);
    mprAssert(qname.space);
    
    if (TYPE(vp)->helpers.deletePropertyByName) {
        return (TYPE(vp)->helpers.deletePropertyByName)(ejs, vp, qname);
    } else {
        slotNum = ejsLookupVar(ejs, vp, qname, &lookup);
        if (slotNum < 0) {
            ejsThrowReferenceError(ejs, "Property \"%@\" does not exist", qname.name);
            return 0;
        }
        return ejsDeleteProperty(ejs, vp, slotNum);
    }
}


/**
    Get a property at a given slot in a variable.
    @return Returns the requested property varaible.
 */
void *ejsGetProperty(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsType     *type;

    mprAssert(ejs);
    mprAssert(vp);

    type = TYPE(vp);
    mprAssert(type->helpers.getProperty);
    return (type->helpers.getProperty)(ejs, vp, slotNum);
}


void *ejsGetPropertyByName(Ejs *ejs, EjsAny *vp, EjsName name)
{
    EjsType     *type;
    int         slotNum;

    mprAssert(ejs);
    mprAssert(vp);

    type = TYPE(vp);

    /*
        WARNING: this is not implemented by most types
     */
    if (type->helpers.getPropertyByName) {
        return (type->helpers.getPropertyByName)(ejs, vp, name);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, vp, name);
    if (slotNum < 0) {
        return 0;
    }
    return ejsGetProperty(ejs, vp, slotNum);
}


EjsTrait *ejsGetPropertyTraits(Ejs *ejs, EjsAny *vp, int slotNum)
{
    mprAssert(TYPE(vp)->helpers.getPropertyTraits);
    return (TYPE(vp)->helpers.getPropertyTraits)(ejs, vp, slotNum);
}


/**
    Return the number of properties in the variable.
    @return Returns the number of properties.
 */
int ejsGetLength(Ejs *ejs, EjsAny *vp)
{
    mprAssert(TYPE(vp)->helpers.getPropertyCount);
    return (TYPE(vp)->helpers.getPropertyCount)(ejs, vp);
}


/**
    Return the name of a property indexed by slotNum.
    @return Returns the property name.
 */
EjsName ejsGetPropertyName(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsType     *type;

    type = TYPE(vp);
    mprAssert(type->helpers.getPropertyName);
    return (type->helpers.getPropertyName)(ejs, vp, slotNum);
}


int ejsPropertyHasTrait(Ejs *ejs, EjsAny *vp, int slotNum, int attributes)
{
    EjsTrait    *trait;

    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);

    if ((trait = ejsGetPropertyTraits(ejs, vp, slotNum)) != 0) {
        return trait->attributes & attributes;
    }
    return 0;
}


/**
    Get a property slot. Lookup a property name and return the slot reference. If a namespace is supplied, the property
    must be defined with the same namespace.
    @return Returns the slot number or -1 if it does not exist.
 */
int ejsLookupProperty(Ejs *ejs, EjsAny *vp, EjsName name)
{
    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(name.name);

    mprAssert(TYPE(vp)->helpers.lookupProperty);
    return (TYPE(vp)->helpers.lookupProperty)(ejs, vp, name);
}


/*
    Invoke an operator.
    vp is left-hand-side
    @return Return a variable with the result or null if an exception is thrown.
 */
EjsAny *ejsInvokeOperator(Ejs *ejs, EjsAny *vp, int opCode, EjsAny *rhs)
{
    mprAssert(vp);

    mprAssert(TYPE(vp)->helpers.invokeOperator);
    return (TYPE(vp)->helpers.invokeOperator)(ejs, vp, opCode, rhs);
}


/*
    Set a property and return the slot number. Incoming slot may be -1 to allocate a new slot.
 */
int ejsSetProperty(Ejs *ejs, EjsAny *vp, int slotNum, EjsAny *value)
{
    mprAssert(vp);

    mprAssert(TYPE(vp)->helpers.setProperty);
    return (TYPE(vp)->helpers.setProperty)(ejs, vp, slotNum, value);
}


/*
    Set a property given a name.
 */
int ejsSetPropertyByName(Ejs *ejs, EjsAny *vp, EjsName qname, EjsAny *value)
{
    int     slotNum;

    mprAssert(ejs);
    mprAssert(vp);

    /*
        WARNING: Not all types implement this
     */
    if (TYPE(vp)->helpers.setPropertyByName) {
        return (TYPE(vp)->helpers.setPropertyByName)(ejs, vp, qname, value);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, vp, qname);
    if (slotNum < 0) {
        slotNum = ejsSetProperty(ejs, vp, -1, value);
        if (slotNum < 0) {
            return EJS_ERR;
        }
        if (ejsSetPropertyName(ejs, vp, slotNum, qname) < 0) {
            return EJS_ERR;
        }
        return slotNum;
    }
    return ejsSetProperty(ejs, vp, slotNum, value);
}


/*
    Set the property name and return the slot number. Slot may be -1 to allocate a new slot.
 */
int ejsSetPropertyName(Ejs *ejs, EjsAny *vp, int slot, EjsName qname)
{
    mprAssert(TYPE(vp)->helpers.setPropertyName);
    return (TYPE(vp)->helpers.setPropertyName)(ejs, vp, slot, qname);
}


int ejsSetPropertyTraits(Ejs *ejs, EjsAny *vp, int slot, EjsType *type, int attributes)
{
    mprAssert(TYPE(vp)->helpers.setPropertyTraits);
    return (TYPE(vp)->helpers.setPropertyTraits)(ejs, vp, slot, type, attributes);
}


/**
    Get a string representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToString(Ejs *ejs, EjsAny *vp)
{
    if (vp == 0 || ejsIsString(ejs, vp)) {
        return (EjsString*) vp;
    }
    return (EjsString*) ejsCast(ejs, vp, ejs->stringType);
}


/**
    Get a numeric representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsNumber *ejsToNumber(Ejs *ejs, EjsAny *vp)
{
    if (vp == 0 || ejsIsNumber(ejs, vp)) {
        return (EjsNumber*) vp;
    }
    if (TYPE(vp)->helpers.cast) {
        return (EjsNumber*) (TYPE(vp)->helpers.cast)(ejs, vp, ejs->numberType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%@\"", TYPE(vp)->qname.name);
    return 0;
}


/**
    Get a boolean representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsBoolean *ejsToBoolean(Ejs *ejs, EjsAny *vp)
{
    if (vp == 0 || ejsIsBoolean(ejs, vp)) {
        return (EjsBoolean*) vp;
    }
    if (TYPE(vp)->helpers.cast) {
        return (EjsBoolean*) (TYPE(vp)->helpers.cast)(ejs, vp, ejs->booleanType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%@\"", TYPE(vp)->qname.name);
    return 0;
}


/**
    Get a Path representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsPath *ejsToPath(Ejs *ejs, EjsAny *vp)
{
    if (vp == 0 || ejsIsPath(ejs, vp)) {
        return (EjsPath*) vp;
    }
    return (EjsPath*) ejsCast(ejs, vp, ejs->pathType);
}


/**
    Get a Uri representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsUri *ejsToUri(Ejs *ejs, EjsAny *vp)
{
    if (vp == 0 || ejsIsUri(ejs, vp)) {
        return (EjsUri*) vp;
    }
    return (EjsUri*) ejsCast(ejs, vp, ejs->uriType);
}


/*
    Fully construct a new object. We create a new instance and call all required constructors.
 */
EjsAny *ejsCreateInstance(Ejs *ejs, EjsType *type, int argc, void *argv)
{
    EjsAny  *vp;

    mprAssert(type);

    vp = ejsCreate(ejs, type, 0);
    if (vp == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (type->constructor.block.pot.isFunction) {
        ejsRunFunction(ejs, (EjsFunction*) type, vp, argc, argv);
    }
    return vp;
}


/************************************* Misc ***********************************/

static void missingHelper(Ejs *ejs, EjsType *type, cchar *helper) 
{
    ejsThrowInternalError(ejs, "The \"%s\" helper is not defined for this type \"%@\"", helper, type->qname.name);
}


static EjsObj *castObj(Ejs *ejs, EjsObj *obj, EjsType *type)
{
    EjsString       *str;
    EjsFunction     *fun;
    EjsObj          *result;
    EjsLookup       lookup;
    
    mprAssert(ejsIsType(ejs, type));

    if (type->hasMeta) {
        return ejsRunFunctionByName(ejs, type, N(EJS_META_NAMESPACE, "cast"), type, 1, &obj);
    }
    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejsCreateBoolean(ejs, 1);

    case ES_Number:
        str = ejsToString(ejs, obj);
        if (str == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        return ejsParse(ejs, str->value, ES_Number);

    case ES_String:
        if (!ejsIsType(ejs, obj) && !ejsIsPrototype(ejs, obj)) {
            if (ejsLookupVar(ejs, obj, EN("toString"), &lookup) >= 0 && lookup.obj != ejs->objectType->prototype) {
                fun = ejsGetProperty(ejs, lookup.obj, lookup.slotNum);
                if (fun && ejsIsFunction(ejs, fun) && fun->body.proc != (EjsFun) ejsObjToString) {
                    result = ejsRunFunction(ejs, fun, obj, 0, NULL);
                    return result;
                }
            }
        }
        if (obj == (EjsObj*) ejs->global) {
            return (EjsObj*) ejsCreateStringFromAsc(ejs, "[object global]");
        } else {
            if (TYPE(obj)->helpers.cast && TYPE(obj)->helpers.cast != (EjsCastHelper) castObj) {
                return (TYPE(obj)->helpers.cast)(ejs, obj, type);
            }
            return (EjsObj*) ejsSprintf(ejs, "[object %@]", TYPE(obj)->qname.name);
        }

    default:
        if (ejsIsA(ejs, obj, type)) {
            return obj;
        }
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsObj *cloneObj(Ejs *ejs, EjsObj *obj, bool deep)
{
    return obj;
}


/*
    Cast the operands depending on the operation code
 */
EjsObj *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);

    case EJS_OP_COMPARE_EQ:  case EJS_OP_COMPARE_NE:
        if (ejsIsNull(ejs, rhs) || ejsIsUndefined(ejs, rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        } else if (ejsIsNumber(ejs, rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNumber(ejs, rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    /* Unary operators */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


EjsAny *ejsInvokeOperatorDefault(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs)
{
    EjsAny      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = ejsCoerceOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }
    /* Types now match */
    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs, (lhs == rhs));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs, !(lhs == rhs));

    /* Unary operators */

    case EJS_OP_COMPARE_NOT_ZERO:
        return ejs->trueValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ejs->falseValue;

    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return ejs->oneValue;

    /* Binary operators */

    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL:
    case EJS_OP_REM: case EJS_OP_OR: case EJS_OP_SHL: case EJS_OP_SHR:
    case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, ejsToNumber(ejs, rhs));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


static int defineProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsName qname, EjsType *propType, int64 attributes, 
    EjsObj *value)
{
    missingHelper(ejs, obj->type, "defineProperty");
    return MPR_ERR_BAD_STATE;
}


static int deleteProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    missingHelper(ejs, obj->type, "deleteProperty");
    return MPR_ERR_BAD_STATE;
}


static int deletePropertyByName(Ejs *ejs, EjsObj *obj, EjsName qname)
{
    missingHelper(ejs, obj->type, "deletePropertyByName");
    return MPR_ERR_BAD_STATE;
}


static EjsObj *getProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    if (obj == 0 || obj == ejs->nullValue || obj == ejs->undefinedValue) {
        ejsThrowReferenceError(ejs, "Object is null");
        return NULL;
    }
    return NULL;
}


static int getPropertyCount(Ejs *ejs, EjsObj *obj)
{
    return 0;
}


static EjsName getPropertyName(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsName     qname;

    qname.name = 0;
    qname.space = 0;
    return qname;
}


static EjsTrait *getPropertyTraits(Ejs *ejs, EjsAny *vp, int slotNum)
{
    return 0;
}


static int lookupProperty(struct Ejs *ejs, EjsObj *obj, EjsName qname)
{
    return -1;
}


static int setProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsObj *value)
{
    if (obj == 0 || obj == ejs->nullValue || obj == ejs->undefinedValue) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    missingHelper(ejs, obj->type, "setProperty");
    return MPR_ERR_BAD_STATE;
}


static int setPropertyName(Ejs *ejs, EjsObj *obj, int slotNum, EjsName qname)
{
    if (obj == 0 || obj == ejs->nullValue || obj == ejs->undefinedValue) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    missingHelper(ejs, obj->type, "setPropertyName");
    return MPR_ERR_BAD_STATE;
}


static int setPropertyTraits(Ejs *ejs, EjsObj *obj, int slot, EjsType *type, int attributes)
{
    if (obj == 0 || obj == ejs->nullValue || obj == ejs->undefinedValue) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    missingHelper(ejs, obj->type, "setPropertyTraits");
    return MPR_ERR_BAD_STATE;
}


void ejsCreateObjHelpers(Ejs *ejs)
{
    EjsHelpers      *helpers;

    helpers = &ejs->objHelpers;
    helpers->cast                   = (EjsCastHelper) castObj;
    helpers->clone                  = (EjsCloneHelper) cloneObj;
    helpers->create                 = (EjsCreateHelper) ejsAlloc;
    helpers->defineProperty         = (EjsDefinePropertyHelper) defineProperty;
    helpers->deleteProperty         = (EjsDeletePropertyHelper) deleteProperty;
    helpers->deletePropertyByName   = (EjsDeletePropertyByNameHelper) deletePropertyByName;
    helpers->getProperty            = (EjsGetPropertyHelper) getProperty;
    helpers->getPropertyCount       = (EjsGetPropertyCountHelper) getPropertyCount;
    helpers->getPropertyName        = (EjsGetPropertyNameHelper) getPropertyName;
    helpers->getPropertyTraits      = (EjsGetPropertyTraitsHelper) getPropertyTraits;
    helpers->lookupProperty         = (EjsLookupPropertyHelper) lookupProperty;
    helpers->invokeOperator         = (EjsInvokeOperatorHelper) ejsInvokeOperatorDefault;
    helpers->setProperty            = (EjsSetPropertyHelper) setProperty;
    helpers->setPropertyName        = (EjsSetPropertyNameHelper) setPropertyName;
    helpers->setPropertyTraits      = (EjsSetPropertyTraitsHelper) setPropertyTraits;
}


/************************************* Misc ***********************************/

EjsName ejsEmptyName(Ejs *ejs, cchar *name)
{
    EjsName     n;

    n.name = ejsCreateStringFromAsc(ejs, name);
    n.space = ejs->emptyString;
    return n;
}


EjsName ejsEmptyWideName(Ejs *ejs, MprChar *name)
{
    EjsName     n;

    n.name = ejsCreateString(ejs, name, strlen(name));
    n.space = ejs->emptyString;
    return n;
}


EjsName ejsName(Ejs *ejs, cchar *space, cchar *name)
{
    EjsName     n;

    n.name = ejsCreateStringFromAsc(ejs, name);
    n.space = (space) ? ejsCreateStringFromAsc(ejs, space) : NULL;
    return n;
}


EjsName ejsWideName(Ejs *ejs, MprChar *space, MprChar *name)
{
    EjsName     n;

    n.name = ejsCreateString(ejs, name, -1);
    n.space = ejsCreateString(ejs, space, -1);
    return n;
}


/*
    Parse a string based on formatting instructions and intelligently create a variable.
    Number formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
        [+|-][DIGITS][.][DIGITS][(e|E)[+|-]DIGITS]
 */
//  MOB -- should 2nd arg be EjsString or MprChar?
EjsAny *ejsParse(Ejs *ejs, MprChar *str, int preferredType)
{
    MprChar     *buf;
    int         type;

    mprAssert(str);

    buf = str;
    type = preferredType;

    //  MOB unicode
    while (isspace((int) *buf)) {
        buf++;
    }    
    if (preferredType == ES_Void || preferredType < 0) {
        if (*buf == '-' || *buf == '+') {
            type = ejs->numberType->id;

        } else if (*buf == '/') {
            type = ES_RegExp;

        } else if (!isdigit((int) *buf) && *buf != '.') {
            if (mcmp(buf, "true") == 0) {
                return ejs->trueValue;

            } else if (mcmp(buf, "false") == 0) {
                return ejs->falseValue;
            }
            type = ES_String;

            if (mcmp(buf, "true") == 0 || mcmp(buf, "false") == 0) {
                type = ES_Boolean;
            } else {
                type = ES_String;
            }

        } else {
            type = ES_Number;
        }
    }
    switch (type) {
    case ES_Object:
    case ES_Void:
    case ES_Null:
    default:
        break;

    case ES_Number:
        return ejsCreateNumber(ejs, parseNumber(ejs, buf));

    case ES_Boolean:
        return ejsCreateBoolean(ejs, parseBoolean(ejs, buf));

#if BLD_FEATURE_REGEXP
    case ES_RegExp:
        return ejsCreateRegExp(ejs, ejsCreateString(ejs, buf, -1));
#endif

    case ES_String:
        if (mcmp(buf, "null") == 0) {
            return ejsCreateNull(ejs);

        } else if (mcmp(buf, "undefined") == 0) {
            return ejsCreateUndefined(ejs);
        }
        return ejsCreateString(ejs, buf, wlen(buf));
    }
    return ejsCreateUndefined(ejs);
}


/*
    Convert the variable to a number type. Only works for primitive types.
 */
static bool parseBoolean(Ejs *ejs, MprChar *s)
{
    if (s == 0 || *s == '\0') {
        return 0;
    }
    if (mcmp(s, "false") == 0 || mcmp(s, "FALSE") == 0) {
        return 0;
    }
    return 1;
}


/*
    Convert the string buffer to a Number.
 */
static MprNumber parseNumber(Ejs *ejs, MprChar *str)
{
    MprNumber   n;
    MprChar     *cp, *sp;
    int64       num;
    char        nbuf[32], *dp;
    int         radix, c, negative;

    mprAssert(str);

    num = 0;
    negative = 0;

    if (*str == '-') {
        str++;
        negative = 1;
    } else if (*str == '+') {
        str++;
    }
    if (*str != '.' && !isdigit((int) *str)) {
        return ejs->nanValue->value;
    }
    /*
        Floatng format: [DIGITS].[DIGITS][(e|E)[+|-]DIGITS]
     */
    if (!(*str == '0' && tolower((int) str[1]) == 'x')) {
        for (cp = str; *cp; cp++) {
            if (*cp == '.' || tolower((int) *cp) == 'e') {
                //MOB OPT
                for (sp = str, dp = nbuf; *str && dp < &nbuf[sizeof(nbuf) - 1]; ) {
                    *dp++ = *sp++;
                }
                *dp = '\0';
                n = atof(nbuf);
                if (negative) {
                    n = (0.0 - n);
                }
                return n;
            }
        }
    }

    /*
        Parse an integer. Observe hex and octal prefixes (0x, 0).
     */
    if (*str != '0') {
        /*
         *  Normal numbers (Radix 10)
         */
        while (isdigit((int) *str)) {
            num = (*str - '0') + (num * 10);
            str++;
        }
    } else {
        str++;
        if (tolower((int) *str) == 'x') {
            str++;
            radix = 16;
            while (*str) {
                c = tolower((int) *str);
                if (isdigit(c)) {
                    num = (c - '0') + (num * radix);
                } else if (c >= 'a' && c <= 'f') {
                    num = (c - 'a' + 10) + (num * radix);
                } else {
                    break;
                }
                str++;
            }

        } else{
            radix = 8;
            while (*str) {
                c = tolower((int) *str);
                if (isdigit(c) && c < '8') {
                    num = (c - '0') + (num * radix);
                } else {
                    break;
                }
                str++;
            }
        }
    }

    if (negative) {
        return (MprNumber) (0 - num);
    }
    return (MprNumber) num;
}


MprNumber ejsGetNumber(Ejs *ejs, EjsAny *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(ejs, vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, vp));
    return (vp) ? ((EjsNumber*) (vp))->value: 0;
}


bool ejsGetBoolean(Ejs *ejs, EjsAny *vp)
{
    mprAssert(vp);
    if (!ejsIsBoolean(ejs, vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->booleanType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsBoolean(ejs, vp));
    return (vp) ? ((EjsBoolean*) (vp))->value: 0;
}


int ejsGetInt(Ejs *ejs, EjsAny *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(ejs, vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, vp));
    return (vp) ? ((int) (((EjsNumber*) (vp))->value)): 0;
}


double ejsGetDouble(Ejs *ejs, EjsAny *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(ejs, vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, vp));
    return (vp) ? ((double) (((EjsNumber*) (vp))->value)): 0;
}


void ejsMarkName(EjsName *qname)
{
    mprMark(qname->name);
    mprMark(qname->space);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    @end
 */