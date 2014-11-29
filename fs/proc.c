#include <errno.h>
#include <sys/stat.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/segment.h>  
#include <stdarg.h>
#define BUFFERSIZE 4096
char psbuffer[BUFFERSIZE];
char hdbuffer[BUFFERSIZE];
char inbuffer[BUFFERSIZE];
int psflag = 1,hdflag = 1,inflag = 1;
int inode_hasprint = 0;
int reset_inode = 0;
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args; int i;
	va_start(args, fmt);
	i=vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}

int proc_read(int idev,int dev,char * buf,int count,off_t * pos){
	int i,j,k;
	if(dev == 0){ //psinfo
		char* offpsbuffer = psbuffer;
		struct task_struct * p;
		int offset = 0;
			offset = sprintf(offpsbuffer,"%s\t%s\t%s\t%s\t%s\n","pid","state","father","counter","start_time");
			offpsbuffer += offset;
			for(i = 0;i <= NR_TASKS;i++){
				p = task[i];
				if(p == NULL) break;
				offset = sprintf(offpsbuffer,"%d\t%d\t%d\t%d\t%ld\n",p->pid,p->state,p->father,p->counter,p->start_time);
				offpsbuffer += offset;
			}
		int total = 0;
		for(i = 0; i < count; i++){
			if(psbuffer[i + *pos] == '\0') break;
			total ++;
			put_fs_byte(psbuffer[i + *pos],&buf[i + *pos]);
		}
		*pos = *pos + total;
		return total;
	}else if(dev == 1){ //hdinfo
		char* offhdbuffer = hdbuffer;
		int offset = 0;
		struct super_block * sb; 
		sb=get_super(idev);
 		struct buffer_head * bh;
		int total_blocks = sb->s_nzones;  
		int total_inodes = sb->s_ninodes;
		int free_blocks = 0;
		int used_blocks = 0;   
		int used_inodes = 0;
		int free_inodes = 0;
		int fsb = 1; //逻辑位图与节点位图第一位都不用
		for(i = 0; i < sb->s_zmap_blocks;i++){
			bh = sb->s_zmap[i];
			for(j = 0; j < 1024; j++){
				for(k = 0; k < 8;k++){
					if(fsb) fsb = 0;
					else{
						if((1<<k & bh->b_data[j]) == 0)
							free_blocks++;
					}
				}
			}
		}
		fsb = 1;
		for(i = 0; i < sb->s_imap_blocks;i++){
			bh = sb->s_imap[i];
			for(j = 0; j < 1024; j++){
				for(k = 0; k < 8;k++){
					if(fsb) fsb = 0;
					else{
						if((1<<k & bh->b_data[j]) == 0)
							free_inodes++;
					}
				}
			}
		}
		used_blocks = total_blocks - free_blocks;
		used_inodes = total_inodes - free_inodes;
		offset = sprintf(offhdbuffer,"%s%d%s\n","total_blocks:",total_blocks,";");
		offhdbuffer += offset;
		offset = sprintf(offhdbuffer,"%s%d%s\n","free_blocks:",free_blocks,";");
		offhdbuffer += offset;
		offset = sprintf(offhdbuffer,"%s%d%s\n","used_blocks:",used_blocks,";");
		offhdbuffer += offset;
   		offset = sprintf(offhdbuffer,"%s%d%s\n","total_inodes:",total_inodes,";");
		offhdbuffer += offset;
		offset = sprintf(offhdbuffer,"%s%d%s\n","free_inodes:",free_inodes,";");
		offhdbuffer += offset;
		offset = sprintf(offhdbuffer,"%s%d%s\n","used_inodes:",used_inodes,";");
		offhdbuffer += offset;

		int total = 0;
   		for(i = 0; i < count; i++){
			if(hdbuffer[i + *pos] == '\0') break;  
			total ++;
			put_fs_byte(hdbuffer[i + *pos],&buf[i + *pos]);
		}
		*pos = *pos + total;
		return total;

	}else if(dev == 2){ //inodeinfo
		if(reset_inode){
			reset_inode = 0;
			return 0;
		}
		char* offinbuffer = inbuffer;
		int offset = 0;
		struct super_block * sb; 
		sb=get_super(idev);
		struct m_inode * inr; 
		int pinodenum = 100;
		*pos = 0;
		if(inode_hasprint != pinodenum){
			inode_hasprint ++;
			inr = iget(sb->s_dev,inode_hasprint);
			offset = sprintf(offinbuffer,"%s%d%s%d\n","inr:",inode_hasprint,";zone[0]:",inr->i_zone[0]);
			offinbuffer += offset;
			iput(inr);		
		}
		int total = 0;
		for(i = 0; i < count; i++){
			if(inbuffer[i + *pos] == '\0'){
				if(inode_hasprint >= pinodenum){
					inode_hasprint = 0;
					if(total) reset_inode = 1;
				}
				break;
			}else{
				total ++;
				put_fs_byte(inbuffer[i + *pos],&buf[i + *pos]);  
			}
			
		}
		*pos = *pos + total;
		return total;
	}
}