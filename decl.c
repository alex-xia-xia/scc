#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "sizes.h"
#include "cc.h"
#include "tokens.h"
#include "syntax.h"
#include "symbol.h"

char parser_out_home;


static void declarator(void);

static struct symbol *newiden(char *s, unsigned char key)
{
	register struct symbol *sym = lookup(yytext, yyhash);

	if (!sym)
		sym = install(yytext, yyhash);
	else if (sym->level == nested_level)
		error("redeclaration of '%s'", yytext);
	return sym;
}

static void dirdcl(void)
{
	if (accept('(')) {
		declarator();
		expect(')');
	} else if (yytoken == IDEN) {
		newiden(yytext, yyhash);
		next();
	} else {
		error("expected '(' or identifier before of '%s'", yytext);
	}

	for (;;) {
		if (accept('(')) {
			next();
			pushtype(FTN);
			if (accept(')'))
				; /* TODO: k&r function */
			else
				/* TODO: prototyped function */;
			continue;
		} else if (accept('[')) {
			pushtype(ARY);
			if (accept(']'))
				; /* TODO: automatic size array */
			else
				/* TODO: specify size of array */;
			continue;
		} else {
			return;
		}
	}
}

static unsigned char spec(register struct ctype *cp)
{
	register unsigned char sign, n;

	for (n = sign = 0; ; ++n, next()) {
		switch (yytoken) {
		case TYPEDEF:  case EXTERN: case STATIC: case AUTO:
		case REGISTER: case CONST:  case VOLATILE:
			ctype(cp, yytoken);
			break;
		case UNSIGNED:
			cp->c_unsigned = 1;
		case SIGNED:
			if (sign == yytoken)
				goto duplicated;
			if (sign)
				goto signed_and_unsigned;
			sign = yytoken;
			break;
		case VOID:   case CHAR:   case SHORT:  case INT:
		case FLOAT:  case DOUBLE: case LONG:   case BOOL:
			cp->type = btype(cp->type, yytoken);
			break;
		case STRUCT:    /* TODO */
		case UNION:	/* TODO */
		case ENUM:	/* TODO */
		default:
			if (!cp->type && sign)
				cp->type = INT;
			return n;
		}
	}
duplicated:
	error("duplicated '%s'", yytext);
signed_and_unsigned:
	error("both 'signed' and 'unsigned' in declaration specifiers");
}

static void declarator(void)
{
	unsigned char qlf[NR_DECLARATORS];
	register unsigned char *bp, *lim;

	lim = &qlf[NR_DECLARATORS];
	for (bp = qlf; yytoken == '*' && bp != lim; next()) {
		*bp++ = PTR;
		while (bp != lim) {
			next();
			switch (yytoken) {
			case CONST: case VOLATILE: case RESTRICT:
				*bp++ = yytoken;
				break;
			default:
				goto continue_outer;
			}
		}
	continue_outer: ;
	}
	if (bp == lim)
		error("Too much type declarators");

	dirdcl();

	for (lim = bp - 1, bp = qlf; bp < lim; ++bp)
		pushtype(*bp);
}

static unsigned char listdcl(register struct ctype *tp)
{
	do {
		register  struct ctype *new;

		declarator();
		new = decl_type(tp);
		if (!new->type) {
			warning_error(user_opt.implicit_int,
				      "type defaults to 'int' in declaration of '%s'",
				      yytext);
		} else if (new->type == FTN && yytoken == '{') {
			compound();
			return 0;
		}
	} while (accept(','));

	return 1;
}

unsigned char decl(void)
{
	register struct ctype *tp;

	if (accept(';'))
		return 1;
	tp = newctype();

	if (!spec(tp)) {
		if (nested_level != 0)
			return 0;
		warning("data definition has no type or storage class");
	}
	if (yytoken == ';') {
		warning_error(user_opt.useless_typename,
			      "useless type name in empty declaration");
	} else if (listdcl(tp)) { /* in case of not being a function */
		expect(';');
	}

	delctype(tp);
	return 1;
}

void type_name()
{
}
