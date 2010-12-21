/**
    ejsNull.c - Ejscript Null class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/
/*
    Cast the null operand to a primitive type
 */

static EjsObj *castNull(Ejs *ejs, EjsObj *vp, EjsType *type)
{
    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejs->falseValue;

    case ES_Number:
        return (EjsObj*) ejs->zeroValue;

    case ES_Object:
    default:
        /*
            Cast null to anything else results in a null
         */
        return vp;

    case ES_String:
        return (EjsObj*) ejsCreateStringFromAsc(ejs, "null");
    }
}


static EjsObj *coerceNullOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {

    case EJS_OP_ADD:
        if (!ejsIsNumber(ejs, rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        /* Fall through */

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);

    /*
        Comparision
     */
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNumber(ejs, rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);
        } else if (ejsIsString(ejs, rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_COMPARE_NE:
        if (ejsIsUndefined(ejs, rhs)) {
            return (EjsObj*) ejs->falseValue;
        }
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_EQ:
        if (ejsIsUndefined(ejs, rhs)) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokeNullOperator(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coerceNullOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    /*
        Types now match. Both left and right types are both "null"
     */
    switch (opcode) {

    /*
        NOTE: strict eq is the same as eq
     */
    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return (EjsObj*) ejs->oneValue;

    /*
        Binary operators. Reinvoke with left = zero
     */
    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
}


/*
    iterator native function get(): Iterator
 */
static EjsObj *getNullIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, np, NULL, 0, NULL);
}


static EjsObj *getNullProperty(Ejs *ejs, EjsNull *unused, int slotNum)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
    return 0;
}


/*********************************** Factory **********************************/
/*
    We dont actually allocate any nulls. We just reuse the singleton instance.
 */

EjsNull *ejsCreateNull(Ejs *ejs)
{
    return (EjsNull*) ejs->nullValue;
}


void ejsCreateNullType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->nullType = ejsCreateNativeType(ejs, N("ejs", "Null"), ES_Null, sizeof(EjsNull), NULL, EJS_OBJ_HELPERS);

    type->helpers.cast             = (EjsCastHelper) castNull;
    type->helpers.getProperty      = (EjsGetPropertyHelper) getNullProperty;
    type->helpers.invokeOperator   = (EjsInvokeOperatorHelper) invokeNullOperator;

    ejs->nullValue = ejsCreate(ejs, type, 0);
    ejsSetName(ejs->nullValue, "null");
}


void ejsConfigureNullType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejs->nullType;
    prototype = type->prototype;

    ejsSetProperty(ejs, ejs->global, ES_null, ejs->nullValue);
    ejsBindMethod(ejs, prototype, ES_Null_iterator_get, getNullIterator);
    ejsBindMethod(ejs, prototype, ES_Null_iterator_getValues, getNullIterator);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.

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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
