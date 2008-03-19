/* spiceman - distributed package management tools
 * Copyright (C) Enno Boland
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software */
int exactmatch(const char *s, struct Package *pkg);
int repomatch(const char *s, struct Package *pkg);
int typematch(const char *s, struct Package *pkg);
void unique(const char action, FILE *in, FILE *out);
int wildcardmatch(const char *s, struct Package *pkg);
int filter(int argc, char *argv[], FILE *in, FILE *out);
void filter_help();
