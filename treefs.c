/*
 * treefs.c
 *
 *  Created on: 29/09/2014
 *      Author: hferreira
 */

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#include "dtc.h"

/*Cria o path dos arquivos */
static void create_dir(char * dir_path);
static char * get_full_path(char * base_path, char * extension, bool has_separetor);
static struct node * get_sibling (struct node * node);
static struct node *get_children (struct node * node);
static struct property * get_next_property (struct node * node, struct property * property);
static void write_property(char * node_path, struct property * property);
static void write_name_property(struct node * node, char * node_path);

void write_fstree (char * path, struct node * node);

char * tree_path = "";

static char * get_full_path(char * base_path, char * extension, bool has_separetor)
{
	char * separator = "/";
	char * path;
	int len;

	if(has_separetor)
	{
		len = strlen(base_path) + strlen(extension) + 2;
		/*allocates memory for the fullpath of the file*/
		path = xmalloc(len);
		memset(path, 0, len);

		path = strcat(path, base_path);
		path = strcat(path, separator);
		path = strcat(path, extension);
	}
	else
	{
		len = strlen(base_path) + strlen(extension) + 1;
		/*allocates memory for the fullpath of the dir*/
		path = xmalloc(len);
		memset(path, 0, len);

		path = strcat(path, base_path);
		path = strcat(path, extension);

	}
	return path;

}


static void create_dir(char * dir_path)
{
	struct stat st;

	/* check if dir exists, if don't create it */
	if (lstat(dir_path, &st) < 0)
	{
		mkdir(dir_path, 0775);
	}
	/*if it is not a dir, dies */
	else if (!S_ISDIR(st.st_mode))
	{
		die("%s exists and is not a dir\n", dir_path);
	}

}

static struct node * get_sibling (struct node * node)
{
	struct node * sibling = NULL;

	if(node->next_sibling != NULL)
	{
		sibling = node->next_sibling;
	}

	return sibling;
}



static struct node *get_children (struct node * node)
{
	struct node * children = NULL;

	if(node->children != NULL)
	{
		children = node->children;
	}

	return children;
}

static struct property * get_next_property (struct node * node, struct property * property)
{
	struct property * next_property = NULL;

	/*get the first property*/
	if(property == NULL)
	{
		next_property = node->proplist;
	}
	/*get the next property from the list*/
	else if(property->next != NULL)
	{
		next_property = property->next;
	}

	return next_property;
}

static void write_property(char * node_path, struct property * property)
{
	FILE * file = NULL;
	char * file_path;

	if(property != NULL)
	{

		file_path = get_full_path(node_path, property->name, true);



		/*TODO check errors */
		file = fopen(file_path, "w");

		fwrite(property->val.val, property->val.len, 1, file);
		fclose(file);

		chmod(file_path, 0644);

		free(file_path);

	}

}

static void write_name_property(struct node * node, char * node_path)
{
	int len;
	char * name = NULL;
	if(node->name == NULL)
	{
		die("Node %s, does not have a name", node_path);
	}

	else
	{
		FILE * file;
		char * prop_name = "name";
		char * name_path;


		name_path = xmalloc((strlen(node_path) + 1));
		name_path = strcpy(name_path,node_path);

		name_path = get_full_path(name_path, prop_name, true);
		file = fopen(name_path, "w");

		len = strlen(node->name) + 1;
		name = xmalloc(len);
		name = strncpy(name, node->name, len);


		char * substring;
		int index = 0;

		substring = strstr(name, "@");

		if(substring != NULL)
		{
			index = (substring - name);

			free(name);

			name = xmalloc((index + 1));
			name = strncpy(name, node->name, (index));
			name[index] = '\0';
		}



		fwrite(name, (strlen(name) + 1), 1, file);
		fclose(file);

		chmod(name_path, 0644);

		free(name_path);
		free(name);

	}


}

void write_fstree (char * path, struct node * node)
{
	char * node_path;
	char * base_path = "";
	struct property * prop = NULL;
	struct node * children = NULL;
	struct node * sibling = NULL;




	base_path = get_full_path(base_path, path, false);


	node_path = get_full_path(base_path, node->fullpath, false);


	create_dir(node_path);

	write_name_property(node, node_path);

	do
	{
		prop = get_next_property (node, prop);
		write_property(node_path, prop);

	}while (prop != NULL);


	children = get_children(node);

	if(children != NULL)
	{
		write_fstree(base_path, children);

	}

	sibling = get_sibling(node);

	if(sibling != NULL)
	{
		node_path = get_full_path(node_path, base_path, false);
		write_fstree(base_path, sibling);

	}

	free(base_path);

	free(node_path);
}


void dt_to_fs(const char * path, struct boot_info *bi)
{
	struct node *dt;
	dt = bi->dt;

	if(path == NULL)
	{
		path = ".";
	}

	tree_path = xmalloc((strlen(path) +1));
	tree_path = strncpy(tree_path, path,(strlen(path)+1));

	create_dir(tree_path);

	write_fstree(tree_path, dt);
	free(tree_path);

}
