/* --------------------------------------------------------------------------
 * Strongly connected components algorithm for static.c.
 *
 * Hugs 98 is Copyright (c) Mark P Jones, Alastair Reid and the Yale
 * Haskell Group 1994-99, and is distributed as Open Source software
 * under the Artistic License; see the file "Artistic" that is included
 * in the distribution for details.
 *
 * $RCSfile: scc.c,v $
 * $Revision: 1.2 $
 * $Date: 1999/07/28 18:48:19 $
 * ------------------------------------------------------------------------*/

#ifndef SCC_C
#define SCC_C
#define visited(d) (isInt(DEPENDS(d)))		/* binding already visited?*/

static Cell daSccs = NIL;
static Int  daCount;

static Int local sccMin	Args((Int,Int));

static Int local sccMin(x,y)		/* calculate minimum of x,y	   */
Int x, y; {				/* (unless y is zero)		   */
    return (x<=y || y==0) ? x : y;
}
#endif

/* --------------------------------------------------------------------------
 * A couple of parts of this program require an algorithm for sorting a list
 * of values (with some added dependency information) into a list of strongly
 * connected components in which each value appears before its dependents.
 *
 * The algorithm used here is based on those described in:
 * 1) Robert Tarjan, Depth-first search and Linear Graph Algorithms,
 *    SIAM J COMPUT, vol 1, no 2, June 1972, pp.146-160.
 * 2) Aho, Hopcroft and Ullman, Design and Analysis of Algorithms,
 *    Addison Wesley, 1972.  pp.189-195.
 * The version used here probably owes most to the latter presentation but
 * has been modified to simplify the algorithm and improve the use of space.
 *
 * This would probably have been a good application for C++ templates ...
 * ------------------------------------------------------------------------*/

static Int local LOWLINK Args((Cell));	/* local function		   */
static Int local LOWLINK(v)		/* calculate `lowlink' of v	   */
Cell v; {
    Int  low = daCount;
    Int  dfn = daCount; 		/* depth first search no. of v	   */
    List ws  = DEPENDS(v);		/* adjacency list for v		   */

    SETDEPENDS(v,mkInt(daCount++));     /* push v onto stack		   */
    push(v);

    while (nonNull(ws)) {		/* scan adjacency list for v	   */
	Cell w = hd(ws);
	ws     = tl(ws);
	low    = sccMin(low, (visited(w) ? intOf(DEPENDS(w)) : LOWLINK(w)));
    }

    if (low == dfn) {			/* start a new scc?		   */
	List temp=NIL;
	do {				/* take elements from stack	   */
	    SETDEPENDS(top(),mkInt(0));
	    temp = cons(top(),temp);
	} while (pop()!=v);
	daSccs = cons(temp,daSccs);	/* make new strongly connected comp*/
    }

    return low;
}

#ifdef SCC
static List local SCC(bs)		/* sort list with added dependency */
List bs; {				/* info into SCCs		   */
    clearStack();
    daSccs = NIL;			/* clear current list of SCCs	   */

    for (daCount=1; nonNull(bs); bs=tl(bs))	 /* visit each binding	   */
	if (!visited(hd(bs)))
	    LOWLINK(hd(bs));

    return rev(daSccs);			/* reverse to obtain correct order */
}
#endif

#ifdef SCC2				/* Two argument version		   */
static List local SCC2(bs,cs)		/* sort lists with added dependency*/
List bs, cs; {				/* info into SCCs		   */
    clearStack();
    daSccs = NIL;			/* clear current list of SCCs	   */

    for (daCount=1; nonNull(bs); bs=tl(bs))	 /* visit each binding	   */
	if (!visited(hd(bs)))
	    LOWLINK(hd(bs));
    for (; nonNull(cs); cs=tl(cs))
	if (!visited(hd(cs)))
	    LOWLINK(hd(cs));

    return rev(daSccs);			/* reverse to obtain correct order */
}
#endif

/*-------------------------------------------------------------------------*/
