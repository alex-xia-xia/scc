
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../inc/cc.h"
#include "cc1.h"

static void emitbin(unsigned, void *),
            emitcast(unsigned, void *), emitswitch(unsigned, void *),
            emitsym(unsigned, void *), emitfield(unsigned, void *),
            emitexp(unsigned, void *),
            emitsymid(unsigned, void *), emittext(unsigned, void *),
            emitprint(unsigned, void *), emitfun(unsigned, void *),
            emitret(unsigned, void *), emitdcl(unsigned, void *);

char *optxt[] = {
	[OADD] = "+",
	[OSUB] = "-",
	[OMUL] = "*",
	[OINC] = ";+",
	[ODEC] =  ";-",
	[OPTR] = "@",
	[OMOD] = "%",
	[ODIV] = "/",
	[OSHL] = "l",
	[OSHR]  = "r",
	[OLT] = "<",
	[OGT] = ">",
	[OGE] = "]",
	[OLE] =  "[",
	[OEQ] = "=",
	[ONE] = "!",
	[OBAND] = "&",
	[OBXOR]  = "^",
	[OBOR] = "|",
	[OASSIGN] = ":",
	[OA_MUL] = ":*",
	[OA_DIV] = ":/",
	[OA_MOD] = ":%",
	[OA_ADD] = ":+",
	[OA_SUB] = ":-",
	[OA_SHL] = ":l",
	[OA_SHR] = ":r",
	[OA_AND] = ":&",
	[OA_XOR] = ":^",
	[OA_OR] = ":|",
	[OADDR] = "a",
	[ONEG] = "_",
	[OCPL] = "~",
	[OAND] = "y",
	[OOR] = "o",
	[OASK] = "?",
	[OCOMMA] = ",",
	[OLABEL] = "L%d\n",
	[ODEFAULT] = "\tf\tL%d\n",
	[OCASE] = "\tw\tL%d",
	[OSTRUCT] = "S%d\t(\n",
	[OJUMP] = "\tj\tL%d\n",
	[OBRANCH] = "\tj\tL%d",
	[OEFUN] = "}",
	[OESTRUCT] = ")",
	[OELOOP] = "\tb",
	[OBLOOP] = "\td"
};

void (*opcode[])(unsigned, void *) = {
	[OADD] = emitbin,
	[OSUB] = emitbin,
	[OMUL] = emitbin,
	[OINC] = emitbin,
	[ODEC] =  emitbin,
	[OPTR] = emitbin,
	[OMOD] = emitbin,
	[ODIV] = emitbin,
	[OSHL] = emitbin,
	[OSHR]  = emitbin,
	[OLT] = emitbin,
	[OGT] = emitbin,
	[OGE] = emitbin,
	[OLE] =  emitbin,
	[OEQ] = emitbin,
	[ONE] = emitbin,
	[OBAND] = emitbin,
	[OBXOR]  = emitbin,
	[OBOR] = emitbin,
	[OASSIGN] = emitbin,
	[OA_MUL] = emitbin,
	[OA_DIV] = emitbin,
	[OA_MOD] = emitbin,
	[OA_ADD] = emitbin,
	[OA_SUB] = emitbin,
	[OA_SHL] = emitbin,
	[OA_SHR] = emitbin,
	[OA_AND] = emitbin,
	[OA_XOR] = emitbin,
	[OA_OR] = emitbin,
	[OADDR] = emitbin,
	[ONEG] = emitbin,
	[OCPL] = emitbin,
	[OAND] = emitbin,
	[OOR] = emitbin,
	[OCOMMA] = emitbin,
	[OCAST] = emitcast,
	[OSYM] = emitsym,
	[OASK] = emitbin,
	[OCOLON] = emitbin,
	[OFIELD]= emitfield,
	[OEXPR] = emitexp,
	[OLABEL] = emitsymid,
	[ODEFAULT] = emitsymid,
	[OCASE] = emitsymid,
	[OSTRUCT] = emitsymid,
	[OJUMP] = emitsymid,
	[OBRANCH] = emitsymid,
	[OEFUN] = emittext,
	[OESTRUCT] = emittext,
	[OELOOP] = emittext,
	[OBLOOP] = emittext,
	[OFUN] = emitfun,
	[ORET] = emitret,
	[ODECL] = emitdcl,
	[OSWITCH] = emitswitch
};

static Node *simple_sub(Node *), *simple_add(Node *);

static Node *(*opsimp[])(Node *) = {
	[OADD] = simple_add,
	[OSUB] = simple_sub

};


void
freetree(Node *np)
{
	if (!np)
		return;
	freetree(np->left);
	freetree(np->right);
	free(np);
}

static void
emitnode(Node *np)
{
	if (np)
		(*opcode[np->op])(np->op, np);
}

void
emit(unsigned op, void *arg)
{
	extern int failure;

	if (failure)
		return;
	(*opcode[op])(op, arg);
}

static void
emitvar(Symbol *sym)
{
	char c;

	if (sym->flags & ISSTATIC)
		c = (sym->flags & ISGLOBAL) ? 'Y' : 'T';
	else if (sym->flags & ISGLOBAL)
		c = 'G';
	else if (sym->flags & ISREGISTER)
		c = 'K';
	else if (sym->flags & ISFIELD)
		c = 'M';
	else if (sym->flags & ISPARAM)
		c = 'P';
	else
		c = 'A';
	printf("%c%d", c, sym->id);
}

static void
emitconst(Node *np)
{
	char *bp, c;
	Symbol *sym = np->sym;

	switch (np->type->op) {
	case INT:
		printf("#%c%x", np->type->letter, sym->u.i);
		break;
	case ARY:
		/*
		 * FIX: At this point we are going to assume
		 * that all the arrays are strings
		 */
		putchar('"');
		for (bp = sym->u.s; c = *bp; ++bp)
			printf("%02X", c & 0xFF);
		break;
	default:
		/* TODO: Handle other kind of constants */
		abort;
	}
}

static void
emitsym(unsigned op, void *arg)
{
	Node *np = arg;
	putchar('\t');
	(np->constant) ? emitconst(np) : emitvar(np->sym);
}

static void
emittype(Type *tp)
{
	putchar(tp->letter);
}

static void
emitdcl(unsigned op, void *arg)
{
	Symbol *sym = arg;

	emitvar(sym);
	putchar('\t');
	emittype(sym->type);
	putchar('\n');
}

static void
emitcast(unsigned op, void *arg)
{
	Node *np = arg, *lp = np->left;

	emitnode(lp);
	printf("\t%c%c", lp->type->letter, np->type->letter);
}

static void
emitbin(unsigned op, void *arg)
{
	Node *np = arg;
	char *s;

	emitnode(np->left);
	emitnode(np->right);
	if ((s = optxt[op]) != NULL)
		printf("\t%s%c", s, np->type->letter);
}

static void
emitexp(unsigned op, void *arg)
{
	Node *np = arg;

	emitnode(np);
	putchar('\n');
	freetree(np);
}

static void
emitfun(unsigned op, void *arg)
{
	Symbol *sym = arg;

	printf("%c%d\tF\t%s\t{\n",
	       sym->flags & ISGLOBAL ? 'G' : 'Y', sym->id, sym->name);
}

static void
emitret(unsigned op, void *arg)
{
	Type *tp = arg;

	fputs("\ty", stdout);
	emittype(tp);
}

static void
emittext(unsigned op, void *arg)
{
	puts(optxt[op]);
}

static void
emitsymid(unsigned op, void *arg)
{
	Symbol *sym = arg;
	printf(optxt[op], sym->id);
}

static void
emitswitch(unsigned op, void *arg)
{
	Caselist *lcase = arg;

	printf("\teI\t#%0x", lcase->nr);
}

void
emitfield(unsigned op, void *arg)
{
	Node *np = arg;

	emitnode(np->left);
	putchar('\t');
	emitvar(np->sym);
}

Node *
node(unsigned op, Type *tp, Node *lp, Node *rp)
{
	Node *np;

	np = xmalloc(sizeof(*np));
	np->op = op;
	np->type = tp;
	np->sym = NULL;
	np->constant = np->symbol = np->lvalue = 0;
	np->left = lp;
	np->right = rp;

	return np;
}

Node *
varnode(Symbol *sym)
{
	Node *np;

	np = node(OSYM, sym->type, NULL, NULL);
	np->lvalue = 1;
	np->constant = 0;
	np->symbol = 1;
	np->sym = sym;
	return np;
}

Node *
constnode(Symbol *sym)
{
	Node *np;

	np = node(OSYM, sym->type, NULL, NULL);
	np->symbol = 1;
	np->constant = 1;
	np->sym = sym;
	return np;
}

Node *
sizeofnode(Type *tp)
{
	Node *np;
	Symbol *sym;

	sym = newsym(NS_IDEN);
	sym->type = sizettype;
	sym->u.i = tp->size;
	return constnode(sym);
}

static Node *
simple_add(Node *np)
{
	Node *lp = np->left, *rp = np->right;
	Type *tp = np->type;
	Symbol *sym, *ls = lp->sym, *rs = rp->sym;

	switch (tp->op) {
	case INT:
		sym = newsym(NS_IDEN);
		sym->type = tp;
		if (tp->sign)
			sym->u.i = ls->u.i + rs->u.i;
		else
			sym->u.u = ls->u.u + rs->u.u;
		freetree(np);
		return constnode(sym);
	default:
		return np;
	}
}

static Node *
simple_sub(Node *np)
{
	Node *lp = np->left, *rp = np->right;
	Type *tp = np->type;
	Symbol *sym, *ls = lp->sym, *rs = rp->sym;

	switch (tp->op) {
	case INT:
		sym = newsym(NS_IDEN);
		sym->type = tp;
		if (tp->sign)
			sym->u.i = ls->u.i - rs->u.i;
		else
			sym->u.u = ls->u.u - rs->u.u;
		freetree(np);
		return constnode(sym);
	default:
		return np;
	}
}

Node *
simplify(Node *np)
{
	if (!np->left->constant || !np->right->constant)
		return np;
	return (*opsimp[np->op])(np);
}
