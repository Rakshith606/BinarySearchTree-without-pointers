#include "header.h"
FILE* init_tree(const char* filename){
	FILE* fp;
	fp = fopen(filename, "rb+");
	if(fp == NULL){
		fp = fopen(filename, "wb+");
		if(fp == NULL){
			return NULL;
		}
		else{
			tree_t header;
			header.free_head=header.root=-1; //initialization 
			fwrite(&header,sizeof(header),1,fp);
		}		
	}
	return fp;	
}	
void close_tree(FILE *fp){
	fclose(fp);
}
void inorder(node_t node,FILE *fp){
	node_t curr=node; //current node
	if(curr.left_offset!=-1){ 				//process left subtree
		fseek(fp,curr.left_offset,SEEK_SET);
		fread(&node,sizeof(node),1,fp);
		inorder(node,fp);
	}
	printf("%d ",curr.key);				//print root
	if(curr.right_offset!=-1){
		fseek(fp,curr.right_offset,SEEK_SET);//process right subtree
		fread(&node,sizeof(node),1,fp);
		inorder(node,fp);
	}
}
void preorder(node_t node,FILE *fp){
	node_t curr=node;
	printf("%d ",curr.key); //print root
	if(curr.left_offset!=-1){
		fseek(fp,curr.left_offset,SEEK_SET);//process left subtree
		fread(&node,sizeof(node),1,fp);
		preorder(node,fp);
	}
	if(curr.right_offset!=-1){
		fseek(fp,curr.right_offset,SEEK_SET);//process right subtree
		fread(&node,sizeof(node),1,fp);
		preorder(node,fp);
	}
}
int get_to_root(FILE *fp,int choice){
	tree_t head;
	fseek(fp,0,SEEK_SET);
	fread(&head,sizeof(head),1,fp);
	if(head.root==-1) //empty tree
		return 0; 
	node_t node;
	fseek(fp,head.root,SEEK_SET); //read root
	fread(&node,sizeof(node),1,fp);
	if(choice)					
		inorder(node,fp);
	else
		preorder(node,fp);
	printf("\b\n"); // \b to remove space after printing last element
	return 0; 
}
void display_inorder(FILE *fp){
	get_to_root(fp,1); //choice 1 denotes preorder in get_to_root
}
void display_preorder(FILE *fp){
	get_to_root(fp,0); //choice 0 denotes inorder in get_to_root
}
void write_node(FILE *fp,node_t node){
	fwrite(&node,sizeof(node),1,fp); //write to location pointed by fp
}
void update_parent(FILE *fp,node_t parent,int curr,int offset,int r){//used while inserting a new key
	fseek(fp,curr-sizeof(parent),SEEK_SET);	//this function updates parent nodes offsets so that it points 
	if(r)										//to the new node
		parent.right_offset=offset;
	else	
		parent.left_offset=offset;
	fwrite(&parent,sizeof(parent),1,fp);
}
void update_head(FILE *fp,tree_t head){
	fseek(fp,0,SEEK_SET);				//update the header node of tree,not root
	fwrite(&head,sizeof(head),1,fp);
}
void write_at_EOF(FILE *fp,node_t node,int r,node_t parent){//as name suggests writes to EOF
	int curr=ftell(fp);				// when free_head = -1 we have no free nodes so only place is
	fseek(fp,0,SEEK_END);			//to write is at EOF
	fwrite(&node,sizeof(node),1,fp); //write new node
	int dest=ftell(fp);
	fseek(fp,curr,SEEK_SET);
	update_parent(fp,parent,curr,dest-sizeof(node),r);//accordingly update parent node
}
void write_to_free_node(FILE *fp,node_t node,node_t temp,tree_t head,int r){
	int parentoffset=ftell(fp)-sizeof(temp); //writes to free location pointed by free_head
	fseek(fp,head.free_head,SEEK_SET);
	node_t free_node;		//before writing to destination read the node and get the next free node
	int destnodeoffset=head.free_head;	//which are linked
	fread(&free_node,sizeof(free_node),1,fp);
	if(r)
		temp.right_offset=destnodeoffset;
	else
		temp.left_offset=destnodeoffset;

	fseek(fp,parentoffset,SEEK_SET);
	write_node(fp,temp);
	fseek(fp,destnodeoffset,SEEK_SET);
	write_node(fp,node);					//please note right offset is used to link free nodes
	head.free_head=free_node.right_offset; //head.free_head now points to next free location
	update_head(fp,head);//update head accordingly
}
void insert_key(int key, FILE *fp){
	tree_t head;
	fseek(fp,0,SEEK_SET);
	fread(&head,sizeof(head),1,fp); //read header
	node_t node;
	node.key=key;	//new node
	node.left_offset=node.right_offset=-1;
	if(head.root==-1){ 	//Empty Tree
		if(head.free_head==-1){ // when both root and free head are -1 we have to write at very beginning
			fseek(fp,sizeof(head),SEEK_SET);
			write_node(fp,node);
			head.root=sizeof(head);
			update_head(fp,head);
		}
		else{
			node_t temp;	//else go to free node location
			fseek(fp,head.free_head,SEEK_SET);	
			fread(&temp,sizeof(temp),1,fp); //read the free node before writing,to get next free node
			fseek(fp,ftell(fp)-sizeof(temp),SEEK_SET);
			write_node(fp,node);//write root node here
			head.root=head.free_head;//head.root points to the above written location's offset
			head.free_head=temp.right_offset; //free_head is updated 
			update_head(fp,head);	//accordingly head is updated
		}
	}
	else{
		node_t temp;
		fseek(fp,head.root,SEEK_SET); //read root node
		fread(&temp,sizeof(temp),1,fp);
		while(1){
			if(temp.key==node.key) //duplicates if found do nothing
				return;
			else if(node.key>temp.key){
				if(temp.right_offset==-1){ //position found to insert
					if(head.free_head==-1) //if no nodes are free in betweem
						write_at_EOF(fp,node,1,temp);	//write at EOF, 1 denotes right child
					else		//else write to free node pointed by head.free_head
						write_to_free_node(fp,node,temp,head,1); // 1 denotes right child in the function
					break;										//weite_to_free_node()
				}
				fseek(fp,temp.right_offset,SEEK_SET);//get next node in tree
				fread(&temp,sizeof(temp),1,fp);
			}
			else{
				if(temp.left_offset==-1){
					if(head.free_head==-1)
						write_at_EOF(fp,node,0,temp); //0 denotes left child
					else{	
						write_to_free_node(fp,node,temp,head,0);
					}
					break;
				}
				fseek(fp,temp.left_offset,SEEK_SET);
				fread(&temp,sizeof(temp),1,fp);	
			}
		}
	}
}
void markfree(node_t temp,FILE *fp,tree_t head){
	temp.right_offset=temp.left_offset=-1; //node which is to be freed,has it's offset set to -1
	if(head.free_head==-1){//first time a node is freed,free_head points to the location
		head.free_head=ftell(fp)-sizeof(temp);
		fseek(fp,head.free_head,SEEK_SET);//go to free node
		write_node(fp,temp);//overwrite the free node location
	}	//RIGHT OFFSET is used to link free nodes
	else{
		int off=ftell(fp);//linking free nodes is done here
		node_t upnode;
		fseek(fp,head.free_head,SEEK_SET); //get to first free node
		fread(&upnode,sizeof(upnode),1,fp);
		while(upnode.right_offset!=-1){//get to last free node
			fseek(fp,upnode.right_offset,SEEK_SET);
			fread(&upnode,sizeof(upnode),1,fp);
		}
		upnode.right_offset=off-sizeof(temp); //update right offset of last free node to current free node
		fseek(fp,ftell(fp)-sizeof(upnode),SEEK_SET);
		write_node(fp,upnode);//update the last free node
		fseek(fp,off-sizeof(temp),SEEK_SET);
		write_node(fp,temp);//update the current free node
	}
	update_head(fp,head);//finally update head 
}
void update_root(tree_t head,node_t root,FILE *fp){ //used in delete function when root has to be deleted
	if(root.left_offset==-1 && root.right_offset==-1){ //if only 1 node is present
		head.root=-1;	//reset root
		if(head.free_head==-1){//update free head so that it points to the root 
			head.free_head=ftell(fp)-sizeof(root);
			update_head(fp,head);//update head 
			return;
		}
	}
	else if(root.left_offset==-1)
		head.root=root.right_offset;
	else
		head.root=root.left_offset;
	markfree(root,fp,head);//here root is marked free 
}
void update_node(node_t root,FILE *fp,node_t prev,int prevoff,tree_t head){
	int curr=ftell(fp);//used while deleting node with 1 subtree
	if(prev.right_offset==curr-sizeof(root))//if node is the right child of parent connect the proper subtree
		prev.right_offset=(root.right_offset==-1)?root.left_offset:root.right_offset;
	else
		prev.left_offset=(root.left_offset==-1)?root.right_offset:root.left_offset;
	fseek(fp,prevoff-sizeof(prev),SEEK_SET); //update parent with new offset
	write_node(fp,prev);
	fseek(fp,curr,SEEK_SET);
	markfree(root,fp,head); //and mark deleted node as free
}
void delete_key(int key, FILE *fp){
	fseek(fp,0,SEEK_SET);
	tree_t head;
	fread(&head,sizeof(head),1,fp);
	if(head.root==-1) //empty tree nothing to delete
		return;
	node_t temp,root,prev;
	int prevoff,parentnodeoffset;
	fseek(fp,head.root,SEEK_SET);//get to root
	fread(&temp,sizeof(temp),1,fp);
	root=temp;
	int found=0;
	while(1){
		if(temp.key==key){
			found=1; 	//if key found 
			break;
		}
		else if(key>temp.key){
			if(temp.right_offset==-1)
				break;
			prev=temp;		//keep track of parent node
			prevoff=ftell(fp);
			fseek(fp,temp.right_offset,SEEK_SET);
			fread(&temp,sizeof(temp),1,fp);
		}
		else{
			if(temp.left_offset==-1)
				break;
			prev=temp;
			prevoff=ftell(fp);
			fseek(fp,temp.left_offset,SEEK_SET);
			fread(&temp,sizeof(temp),1,fp);	
		}
	}
	if(!found)//if key not found
		return;
	if(temp.left_offset==-1 || temp.right_offset==-1){//deleting node with 1 subtree
		if(root.key==temp.key)
			update_root(head,root,fp);	//if root is being deleted 
		else
			update_node(temp,fp,prev,prevoff,head);//non root node being deleted 
	}
	else{//deleting node with both left and right subtree
		node_t curr=temp;
		prevoff=ftell(fp);
		fseek(fp,temp.right_offset,SEEK_SET); 
		fread(&temp,sizeof(temp),1,fp);
		parentnodeoffset=-1;
		while(temp.left_offset!=-1){
			prev=temp;
			parentnodeoffset=ftell(fp);
			fseek(fp,temp.left_offset,SEEK_SET);
			fread(&temp,sizeof(temp),1,fp);	
		}//temp will now point to inorder successor of the node being deleted
		markfree(temp,fp,head); //mark INORDER SUCCESSOR as free not NODE BEING DELETED
		curr.key=temp.key;//INORDER SUCCESSOR KEY IS COPIED INTO NODE BEING DELETED
		if(parentnodeoffset!=-1){
			prev.left_offset=temp.right_offset;
			fseek(fp,parentnodeoffset-sizeof(prev),SEEK_SET);
			write_node(fp,prev);
		}
		else
			curr.right_offset=temp.right_offset;
		fseek(fp,prevoff-sizeof(curr),SEEK_SET);
		write_node(fp,curr);//UPDATE THE NODE WHICH WAS DELETED
	}
}	