#include <stdio.h>
#include <stdlib.h>
#include "header.h"

int main()
{
	FILE *fp;
	fp = init_tree("mytree.dat");
	int key;
	int opt=0;
	while(1)
	{
		switch(opt)
		{
			case 1 : printf("Enter Key To Insert\n");
					 scanf("%d", &key);
					 insert_key(key, fp);
					 break;
			case 2 : scanf("%d", &key);
					 delete_key(key, fp);
					 break;
			case 3 : printf("**********Inorder Begin**********\n");
					 display_inorder(fp);
					 printf("**********Inorder End**********\n");
					 break;
			case 4:
					 printf("**********Preorder Begin**********\n");
					 display_preorder(fp);
					 printf("**********Preorder End**********\n");
					 break;
			case 5:
					printf("Exit \n");
					exit(0);

		}
		printf("Menu\n");
		printf("1 Insert Key\n");
		printf("2 Delete Key\n");
		printf("3 Inorder Traversal\n");
		printf("4 Preorder Traversal\n");
		printf("5 Exit\n");
		scanf("%d", &opt);
	}
	close_tree(fp);
}
