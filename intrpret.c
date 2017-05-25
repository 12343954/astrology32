/*
** Astrolog (Version 5.41) File: intrpret.c
**
** IMPORTANT NOTICE: The graphics database and chart display routines
** used in this program are Copyright (C) 1991-1998 by Walter D. Pullen
** (Astara@msn.com, http://www.magitech.com/~cruiser1/astrolog.htm).
** Permission is granted to freely use and distribute these routines
** provided one doesn't sell, restrict, or profit from them in any way.
** Modification is allowed provided these notices remain with any
** altered or edited versions of the program.
**
** The main planetary calculation routines used in this program have
** been Copyrighted and the core of this program is basically a
** conversion to C of the routines created by James Neely as listed in
** Michael Erlewine's 'Manual of Computer Programming for Astrologers',
** available from Matrix Software. The copyright gives us permission to
** use the routines for personal use but not to sell them or profit from
** them in any way.
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby
** (brianw@sounds.wa.com). Conditions are identical to those above.
**
** The extended accurate ephemeris databases and formulas are from the
** calculation routines in the library SWISS EPHEMERIS and are programmed and
** copyright 1998 by Astrodienst AG.
** The use of that source code is subject to
** the Swiss Ephemeris Public License, available at 
** http://www.astro.ch/swisseph. This copyright notice must not be 
** changed or removed by any user of this program.
**
** Initial programming 8/28,30, 9/10,13,16,20,23, 10/3,6,7, 11/7,10,21/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 12/20/1998.
** Modifications from version 5.40 to 5.41 are by Alois Treindl.
*/

#include "astrolog.h"


#ifdef INTERPRET
/*
******************************************************************************
** Interpretation Routines.
******************************************************************************
*/

/* This function is used by the interpretation routines to print out lines  */
/* of text with newlines inserted just before the end of screen is reached. */

void FieldWord(sz)
char *sz;
{
  static char line[cchSzMax];
  static int cursor = 0;
  int i, j;

  /* Hack: Dump buffer if function called with a null string. */

  if (sz == NULL) {
    line[cursor] = 0;
    PrintSz(line); PrintL();
    cursor = 0;
    return;
  }
  if (cursor)
    line[cursor++] = ' ';
  for (i = 0; (line[cursor] = sz[i]); i++, cursor++)
    ;

  /* When buffer overflows 'n' columns, display one line and start over. */

  while (cursor >= us.nScreenWidth-1) {
    for (i = us.nScreenWidth-1; line[i] != ' '; i--)
      ;
    line[i] = 0;
    PrintSz(line); PrintL();
    line[0] = line[1] = ' ';
    for (j = 2; (line[j] = line[i+j-1]) != 0; j++)
      ;
    cursor -= (i-1);
  }
}


/* Display a general interpretation of what each sign of the zodiac, house, */
/* and planet or object means. This is called to do the -I0 switch table.   */

void InterpretGeneral()
{
  char sz[cchSzDef*2];
  int i;

  PrintSz("Signs of the zodiac represent psychological characteristics.\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%s is", szSignName[i]); FieldWord(sz);
    sprintf(sz, "%s, and", szDesc[i]); FieldWord(sz);
    sprintf(sz, "%s.", szDesire[i]); FieldWord(sz);
    FieldWord(NULL);
  }
  AnsiColor(kDefault);
  PrintSz("\nHouses represent different areas within one's life.\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "The %d%s House is the area of life dealing with",
      i, szSuffix[i]); FieldWord(sz);
    sprintf(sz, "%s.", szLifeArea[i]); FieldWord(sz);
    FieldWord(NULL);
  }
  AnsiColor(kDefault);
  PrintSz("\nPlanets represent various parts of one's mind or self.\n\n");
  for (i = 1; i <= cObjInt; i++) {
    if (ignore[i] || FCusp(i))
      continue;
    AnsiColor(kObjA[i]);
    if (i <= oMoo || (FBetween(i, oNod, oCore)) && (i != oLil || fSouthNode))
      FieldWord("The");
    sprintf(sz, "%s%s%s represents one's",
      i == oNod ? "North " : (i == oFor ? "Part of " : ""), szObjName[i],
      i == 13 ? " Athena" : ""); FieldWord(sz);
    sprintf(sz, "%s.", szMindPart[i]); FieldWord(sz);
    FieldWord(NULL);
  }
  AnsiColor(kDefault);
}


/* Display a general interpretation of what each aspect type means. This */
/* is called when printing the interpretation table in the -I0 switch.   */

void InterpretAspectGeneral()
{
  char sz[cchSzDef*2];
  int i;

  PrintSz("\nAspects are different relationships between planets.\n\n");
  for (i = 1; i <= Min(us.nAsp, cAspectInt); i++) {
    AnsiColor(kAspA[i]);
    sprintf(sz, "When planets are %s, one", szAspectName[i]);
    FieldWord(sz); sprintf(sz, szInteract[i], ""); FieldWord(sz);
    FieldWord("another.");
    if (szTherefore[i][0]) {
      sprintf(sz, "%s.", szTherefore[i]); FieldWord(sz);
    }
    FieldWord(NULL);
  }
  return;
}


/* Print the interpretation of each planet in sign and house, as specified */
/* with the -I switch. This is basically array accessing combining the     */
/* meanings of each planet, sign, and house, and a couple of other things. */

void InterpretLocation()
{
  char sz[cchSzDef*2], c;
  int i, j;

  PrintL();
  for (i = 1; i <= cObjInt; i++) {
    if (ignore[i] || FCusp(i))
      continue;
    AnsiColor(kObjA[i]);
    j = SFromZ(planet[i]); c = Dignify(i, j);
    sprintf(sz, "%s%s%s%s in %s", ret[i] < 0.0 ? "Retrograde " : "",
      i == oNod ? "North " : (i == oFor ? "Part of " : ""), szObjName[i],
      i == 13 ? " Athena" : "", szSignName[j]);
    FieldWord(sz);
    sprintf(sz, "and %d%s House:", inhouse[i], szSuffix[inhouse[i]]);
    FieldWord(sz);
    sprintf(sz, "%s's", szPerson); FieldWord(sz);
    FieldWord(szMindPart[i]); FieldWord("is");
    if (((int)planet[i]) % 30 < 10)
      FieldWord("very");
    sprintf(sz, "%s, and", szDesc[j]); FieldWord(sz);
    sprintf(sz, "%s.", szDesire[j]); FieldWord(sz);
    FieldWord("Most often this manifests");
    if (ret[i] < 0.0 && i != oNod)
      FieldWord("in an independent, backward, introverted manner, and");
    FieldWord("in the area of life dealing with");
    sprintf(sz, "%s.", szLifeArea[inhouse[i]]); FieldWord(sz);

    /* Extra information if planet is in its ruling, falling, etc, sign. */

    if (c == 'R')
      FieldWord("This is a major aspect of their psyche!");
    else if (c == 'F')
      FieldWord("(This bit plays only a minor part in their psyche.)");
    else if (c == 'e')
      FieldWord("It is easy for them to express this part of themself.");
    else if (c == 'd')
      FieldWord("It is difficult for them to express this part of themself.");
    FieldWord(NULL);
  }
}


/* Print an interpretation for a particular aspect in effect in a chart. */
/* This is called from the InterpretGrid and ChartAspect routines.       */

void InterpretAspect(x, y)
int x, y;
{
  char sz[cchSzDef*2];
  int n;

  n = grid->n[x][y];
  if (n < 1 || n > cAspectInt ||
    FCusp(x) || FCusp(y) || x > cObjInt || y > cObjInt)
    return;
  AnsiColor(kAspA[n]);
  sprintf(sz, "%s %s %s: %s's", szObjName[x],
    szAspectName[n], szObjName[y], szPerson);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, szInteract[n],
    szModify[Min(abs(grid->v[x][y])/150, 2)][n-1]);
  FieldWord(sz);
  sprintf(sz, "their %s.", szMindPart[y]); FieldWord(sz);
  if (szTherefore[n][0]) {
    sprintf(sz, "%s.", szTherefore[n]); FieldWord(sz);
  }
  FieldWord(NULL);
}


/* Print the interpretation of each aspect in the aspect grid, as specified */
/* with the -g -I switch. Again, this is done by basically array accessing  */
/* of the meanings of the two planets in aspect and of the aspect itself.   */

void InterpretGrid()
{
  int i, j;

  for (i = 1; i < cObjInt; i++) if (!ignore[i] && !FCusp(i))
    for (j = i+1; j <= cObjInt; j++) if (!ignore[j] && !FCusp(i))
      InterpretAspect(i, j);
}


/* Print an interpretation for a particular midpoint in effect in a chart. */
/* This is called from the ChartMidpoint routine.                          */

void InterpretMidpoint(x, y)
int x, y;
{
  char sz[cchSzDef*2];
  int n, i;

  if (FCusp(x) || FCusp(y) || x > cObjInt || y > cObjInt)
    return;
  n = grid->n[y][x];
  AnsiColor(kSignA(n));
  sprintf(sz, "%s midpoint %s in %s: The merging of %s's",
    szObjName[x], szObjName[y], szSignName[n], szPerson0);
  FieldWord(sz); FieldWord(szMindPart[x]);
  FieldWord("with their"); FieldWord(szMindPart[y]);
  FieldWord("is");
  if (grid->v[y][x]/60 < 10)
    FieldWord("very");
  sprintf(sz, "%s, and", szDesc[n]); FieldWord(sz);
  sprintf(sz, "%s.", szDesire[n]); FieldWord(sz);
  FieldWord("Most often this manifests in");
  if (ret[x]+ret[y] < 0.0 && x != oNod && y != oNod)
    FieldWord("an independent, backward, introverted manner, and");
  FieldWord("the area of life dealing with");
  i = HousePlaceIn(ZFromS(n) + (real)grid->v[y][x]/60.0);
  sprintf(sz, "%s.", szLifeArea[i]); FieldWord(sz);
  FieldWord(NULL);
}


/* This is a subprocedure of ChartInDaySearch(). Print the interpretation    */
/* for a particular instance of the various exciting events that can happen. */

void InterpretInDay(source, aspect, dest)
int source, aspect, dest;
{
  char sz[cchSzDef*2];

  if (source > cObjInt || dest > cObjInt)
    return;

  /* Interpret object changing direction. */

  if (aspect == aDir) {
    AnsiColor(kObjA[source]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    FieldWord("will tend to manifest in");
    FieldWord(dest ? "an independent, backward, introverted" :
      "the standard, direct, open");
    FieldWord("manner.");
    FieldWord(NULL);

  /* Interpret object entering new sign. */

  } else if (aspect == aSig) {
    AnsiColor(kObjA[source]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, "will be %s,", szDesc[dest]);
    FieldWord(sz);
    sprintf(sz, "and it %s.", szDesire[dest]); FieldWord(sz);
    FieldWord(NULL);

  /* Interpret aspect between transiting planets. */

  } else if (aspect > 0 && aspect <= cAspectInt) {
    AnsiColor(kAspA[aspect]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, szInteract[aspect], szModify[1][aspect-1]);
    FieldWord(sz);
    sprintf(sz, "energies of %s.", szMindPart[dest]); FieldWord(sz);
    if (szTherefore[aspect][0]) {
      if (aspect > aCon) {
        sprintf(sz, "%s.", szTherefore[aspect]); FieldWord(sz);
      } else
        FieldWord("They will affect each other prominently.");
    }
    FieldWord(NULL);
  }
}


/* This is a subprocedure of ChartTransitSearch(). Print the interpretation */
/* for a particular transit of a planet to a natal object of a chart.       */

void InterpretTransit(source, aspect, dest)
int source, aspect, dest;
{
  char sz[cchSzDef*2];

  if (source <= oCore && dest <= oCore && aspect <= cAspectInt) {
    AnsiColor(kAspA[aspect]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, szInteract[aspect], szModify[1][aspect-1]);
    FieldWord(sz);
    if (source != dest) {
      sprintf(sz, "%s's %s.", szPerson0, szMindPart[dest]);
    } else {
      sprintf(sz, "the same aspect inside %s's makeup.", szPerson0);
    }
    FieldWord(sz);
    if (szTherefore[aspect][0]) {
      if (aspect > aCon) {
        sprintf(sz, "%s.", szTherefore[aspect]); FieldWord(sz);
      } else
        FieldWord("This part of their psyche will be strongly influenced.");
    }
    FieldWord(NULL);
  }
}


/* Print the interpretation of one person's planet in another's sign and    */
/* house, in a synastry chart as specified with the -r switch combined with */
/* -I. This is very similar to the interpretation of the standard -v chart  */
/* in InterpretLocation(), but we treat the chart as a relationship here.   */

void InterpretSynastry()
{
  char sz[cchSzDef*2], c;
  int i, j;

  PrintL();
  for (i = 1; i <= cObjInt; i++) {
    if (ignore[i] || FCusp(i))
      continue;
    AnsiColor(kObjA[i]);
    j = SFromZ(planet[i]); c = Dignify(i, j);
    sprintf(sz, "%s%s%s%s in %s,", ret[i] < 0.0 ? "Retrograde " : "",
      i == oNod ? "North " : (i == oFor ? "Part of " : ""), szObjName[i],
      i == 13 ? " Athena" : "", szSignName[j]);
    FieldWord(sz);
    sprintf(sz, "in their %d%s House:", inhouse[i], szSuffix[inhouse[i]]);
    FieldWord(sz);
    sprintf(sz, "%s's", szPerson2); FieldWord(sz);
    FieldWord(szMindPart[i]); FieldWord("is");
    if (((int)planet[i]) % 30 < 10)
      FieldWord("very");
    sprintf(sz, "%s, and", szDesc[j]); FieldWord(sz);
    sprintf(sz, "%s.", szDesire[j]); FieldWord(sz);
    FieldWord("This");
    if (ret[i] < 0.0 && i != oNod)
      FieldWord(
        "manifests in an independent, backward, introverted manner, and");
    sprintf(sz, "affects %s in the area of life dealing with %s.",
      szPerson1, szLifeArea[inhouse[i]]); FieldWord(sz);

    /* Extra information if planet is in its ruling, falling, etc, sign. */

    if (c == 'R') {
      sprintf(sz, "This is a major aspect of %s's psyche!", szPerson2);
      FieldWord(sz);
    } else if (c == 'F') {
      sprintf(sz, "(This bit plays only a minor part in %s's psyche.)",
        szPerson2);
      FieldWord(sz);
    } else if (c == 'e') {
      sprintf(sz, "%s is affected harmoniously in this way.", szPerson1);
      FieldWord(sz);
    } else if (c == 'd') {
      sprintf(sz, "%s is affected discordantly in this way.", szPerson1);
      FieldWord(sz);
    }
    FieldWord(NULL);
  }
}


/* Print an interpretation for a particular aspect in effect in a comparison */
/* relationship chart. This is called from the InterpretGridRelation and     */
/* the ChartAspectRelation routines.                                         */

void InterpretAspectRelation(x, y)
int x, y;
{
  char sz[cchSzDef*2];
  int n;

  n = grid->n[y][x];
  if (n < 1 || n > cAspectInt ||
    FCusp(x) || FCusp(y) || x > cObjInt || y > cObjInt)
    return;
  AnsiColor(kAspA[n]);
  sprintf(sz, "%s %s %s: %s's", szObjName[x],
    szAspectName[n], szObjName[y], szPerson1);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, szInteract[n],
    szModify[Min(abs(grid->v[y][x])/150, 2)][n-1]);
  FieldWord(sz);
  sprintf(sz, "%s's %s.", szPerson2, szMindPart[y]); FieldWord(sz);
  if (szTherefore[n][0]) {
    if (n != 1) {
      sprintf(sz, "%s.", szTherefore[n]); FieldWord(sz);
    } else
      FieldWord("These parts affect each other prominently.");
  }
  FieldWord(NULL);
}


/* Print the interpretation of each aspect in the relationship aspect grid, */
/* as specified with the -r0 -g -I switch combination.                      */

void InterpretGridRelation()
{
  int i, j;

  for (i = 1; i <= cObjInt; i++) if (!ignore[i])
    for (j = 1; j <= cObjInt; j++) if (!ignore[j])
      InterpretAspectRelation(i, j);
}


/* Print the interpretation of a midpoint in the relationship grid, as */
/* specified with the -r0 -m -I switch combination.                    */

void InterpretMidpointRelation(x, y)
int x, y;
{
  char sz[cchSzDef*2];
  int n;

  if (FCusp(x) || FCusp(y) || x > cObjInt || y > cObjInt)
    return;
  n = grid->n[y][x];
  AnsiColor(kSignA(n));
  sprintf(sz, "%s midpoint %s in %s: The merging of %s's",
    szObjName[x], szObjName[y], szSignName[n], szPerson1);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, "with %s's", szPerson2); FieldWord(sz);
  FieldWord(szMindPart[y]); FieldWord("is");
  if (grid->v[y][x]/60 < 10)
    FieldWord("very");
  sprintf(sz, "%s, and", szDesc[n]); FieldWord(sz);
  sprintf(sz, "%s.", szDesire[n]); FieldWord(sz);
  if (cp1.dir[x]+cp2.dir[y] < 0.0 && x != oNod && y != oNod) {
    FieldWord("Most often this manifests in");
    FieldWord("an independent, backward, introverted manner.");
  }
  FieldWord(NULL);
}
#endif /* INTERPRET */


/*
******************************************************************************
** Chart Influence Routines.
******************************************************************************
*/

/* This is a subprocedure of ChartInfluence(). Based on the values in the */
/* array parameter 'value', store numbers in array 'rank' reflecting the  */
/* relative order, e.g. value[x] 2nd greatest array value -> rank[x] = 2. */

void SortRank(value, rank, size)
real *value;
int *rank, size;
{
  int h, i, j, k;

  value[0] = -1.0;
  for (i = 1; i <= size; i++)
    rank[i] = -1;
  for (h = 1, i = 0; h <= size; h++) {
    if (size != cSign && (ignore[h] || !FThing(h)))
      continue;
    i++;
    k = 0;
    for (j = 1; j <= size; j++) {
      if (size != cSign && (ignore[j] || !FThing(j)))
        continue;
      if (value[j] > value[k] && rank[j] < 0)
        k = j;
    }

    /* 'k' is the current position of the 'i'th place planet. */

    rank[k] = i;
  }
}


/* Print out a list of power values and relative rankings, based on the */
/* placements of the planets, and their aspects in the aspect grid, as  */
/* specified with the -j "find influences" switch.                      */

void ChartInfluence()
{
  real power[oNorm+1], power1[oNorm+1], power2[oNorm+1],
    total, total1, total2, x;
  int rank[oNorm+1], rank1[oNorm+1], rank2[oNorm+1], i, j, k, l;
  char sz[cchSzDef], c;

  for (i = 1; i <= oNorm; i++)
    power1[i] = power2[i] = 0.0;
  total = total1 = total2 = 0.0;

  /* First, for each object, find its power based on its placement alone. */

  for (i = 1; i <= oNorm; i++) if (!ignore[i] && FThing(i)) {
    j = SFromZ(planet[i]);
    power1[i] += rObjInf[i];               /* Influence of planet itself. */
    power1[i] += rHouseInf[inhouse[i]];    /* Influence of house it's in. */
    c = Dignify(i, j);
    switch (c) {
    case 'R': x = rObjInf[oNorm+1]; break; /* Planets in signs they rule / */
    case 'e': x = rObjInf[oNorm+2]; break; /* exalted in have influence.   */
    default:  x = 0.0;
    }
    c = Dignify(i, inhouse[i]);
    switch (c) {
    case 'R': x += rHouseInf[cSign+1]; break; /* Item in house aligned with */
    case 'e': x += rHouseInf[cSign+2]; break; /* sign ruled has influence.  */
    default: ;
    }
    power1[i] += x;
    if (i != rules[j])                       /* The planet ruling the sign */
      power1[rules[j]] += rObjInf[i]/2.0;    /* and the house that the     */
    if (i != (j = rules[inhouse[i]]))        /* current planet is in, gets */
      power1[j] += rObjInf[i]/2.0;           /* extra influence.           */
  }
  for (i = 1; i <= cSign; i++) {         /* Various planets get influence */
    j = SFromZ(chouse[i]);               /* if house cusps fall in signs  */
    power1[rules[j]] += rHouseInf[i];    /* they rule.                    */
  }

  /* Second, for each object, find its power based on aspects it makes. */

  if (!FCreateGrid(fFalse))
    return;
  for (j = 1; j <= oNorm; j++) if (!ignore[j] && FThing(j))
    for (i = 1; i <= oNorm; i++) if (!ignore[i] && FThing(i) && i != j) {
      k = grid->n[Min(i, j)][Max(i, j)];
      if (k) {
        l = grid->v[Min(i, j)][Max(i, j)];
        power2[j] += rAspInf[k]*rObjInf[i]*
          (1.0-RAbs((real)l)/60.0/GetOrb(i, j, k));
      }
    }

  /* Calculate total power of each planet. */

  for (i = 1; i <= oNorm; i++) if (!ignore[i] && FThing(i)) {
    power[i] = power1[i]+power2[i]; total1 += power1[i]; total2 += power2[i];
  }
  total = total1+total2;

  /* Finally, determine ranks of the arrays, then print everything out. */

  SortRank(power1, rank1, oNorm); SortRank(power2, rank2, oNorm);
  SortRank(power, rank, oNorm);
  PrintSz("  Planet:    Position      Aspects    Total Rank  Percent\n");
  for (i = 1; i <= oNorm; i++) if (!ignore[i] && FThing(i)) {
    AnsiColor(kObjA[i]);
    sprintf(sz, "%8.8s: ", szObjName[i]); PrintSz(sz);
    sprintf(sz, "%6.1f (%2d) +%6.1f (%2d) =%7.1f (%2d) /%6.1f%%\n",
      power1[i], rank1[i], power2[i], rank2[i], power[i], rank[i],
      total > 0.0 ? power[i]/total*100.0 : 0.0); PrintSz(sz);
  }
  AnsiColor(kDefault);
  sprintf(sz, "   Total: %6.1f      +%6.1f      =%7.1f      / 100.0%%\n",
    total1, total2, total); PrintSz(sz);

  /* Now, print out a list of power values and relative rankings, based on  */
  /* the power of each sign of the zodiac, as indicated by the placement of */
  /* the planets above, in the chart, as specified with the -j0 switch.     */

  if (!us.fInfluenceSign)
    return;
  for (i = 1; i <= cSign; i++)
    power1[i] = 0.0;

  /* For each sign, determine its power based on the power of the object. */

  for (i = 1; i <= oNorm; i++) if (!ignore[i] && FThing(i)) {
    power1[SFromZ(planet[i])] += power[i] / 2.0;
    power1[inhouse[i]]        += power[i] / 4.0;
    power1[ruler1[i]]         += power[i] / 3.0;
    if (ruler2[i])
      power1[ruler2[i]]       += power[i] / 4.0;
  }
  if (!fSouthNode && !ignore[oNod]) {
    power1[Mod12(SFromZ(planet[oNod])+6)] += power[oNod] / 2.0;  /* South */
    power1[Mod12(inhouse[oNod]+6)]        += power[oNod] / 4.0;  /* Node. */
  }
  for (i = cThing+1; i <= oCore; i++) if (!ignore[i]) {
    power1[SFromZ(planet[i])] += rObjInf[i];
  }

  total1 = 0.0;
  for (i = 1; i <= cSign; i++)
    total1 += power1[i];
  for (i = 1; i <= cSign; i++) if (total1 > 0.0)
    power1[i] *= total/total1;
  total1 = total;

  /* Again, determine ranks in the array, and print everything out. */

  SortRank(power1, rank1, cSign);
  PrintSz(
    "\n       Sign:  Power Rank  Percent  -   Element  Power  Percent\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%11.11s: ", szSignName[i]); PrintSz(sz);
    sprintf(sz, "%6.1f (%2d) /%6.1f%%", power1[i],
      rank1[i], total1 > 0.0 ? power1[i]/total1*100.0 : 0.0); PrintSz(sz);
    if (i <= 4) {
      sprintf(sz, "  -%9.7s:", szElem[i-1]); PrintSz(sz);
      total2 = 0.0;
      for (j = 1; j < cSign; j += 4)
        total2 += power1[i-1+j];
      sprintf(sz, "%7.1f /%6.1f%%", total2,
        total1 > 0.0 ? total2/total1*100.0 : 0.0); PrintSz(sz);
    } else if (i == 6) {
      AnsiColor(kDefault);
      PrintSz("  -      Mode  Power  Percent");
    } else if (i >= 7 && i <= 9) {
      AnsiColor(kModeA(i-7));
      sprintf(sz, "  -%9.8s:", szMode[i-7]); PrintSz(sz);
      total2 = 0.0;
      for (j = 1; j < cSign; j += 3)
        total2 += power1[i-7+j];
      sprintf(sz, "%7.1f /%6.1f%%", total2,
        total1 > 0.0 ? total2/total1*100.0 : 0.0); PrintSz(sz);
    }
    PrintL();
  }
  AnsiColor(kDefault);
  sprintf(sz, "      Total:%7.1f      / 100.0%%\n", total1); PrintSz(sz);
}

/* intrpret.c */
