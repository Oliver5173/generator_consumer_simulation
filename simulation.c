#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#define MAXOUTPUT 1000000000
/*define the input buffer*/
typedef struct 
{
	int queue[11];
	int head; 
	int tail;
} buffer_input,buffer_output;

/*mutex for input bufifer and output buffer*/
pthread_mutex_t wait_mutex,tool_mutex,input_mutex,output_mutex;

/*input buffer*/
buffer_input *ibuffer;
buffer_output *oqueue;
int num_op,num_tool,num_wait,num_deadlock;
int num_material[4];

int num_material_used[4]; /*if product abandoned, the material not used*/
/*since output buffer is unlimited and won't affect other function of this program, here just use an array to record the number of every product in the output buffer*/
int obuffer[3];
int can_produce[3]; /*whether this product can be produced this time*/

int genrec_buffer[3]; /*record the most recently material produced*/
pthread_cond_t condc0, condc, condp0, condp, condt;
int pause_signal;
/*check input buffer: 0--empty, positive number-nurmal, -1-full*/
int check_cangen(int gennow)
{
	int cnt=0;
	if(gennow==genrec_buffer[0]) return 0; /*two same material*/
	if(gennow==genrec_buffer[1] && genrec_buffer[0]==genrec_buffer[2]) return 0; /* if generate ABAB, will likely to cause two same product*/
	if(num_material_used[gennow]-num_material_used[1]>5 || num_material_used[gennow]-num_material_used[2]>5 || num_material_used[gennow]-num_material_used[3]>5) /*Need to slow down!*/
	{
		if(num_material_used[gennow]>num_material_used[1]) cnt++;
		if(num_material_used[gennow]>num_material_used[2]) cnt++;
		if(num_material_used[gennow]>num_material_used[3]) cnt++;
		if(cnt==2)/*it's the material of maximum number*/
		{
			/*must generate this or deadlock occur;*/
			if(genrec_buffer[1]!=gennow && genrec_buffer[2]!=gennow) return 1;
			return 0;
		}
	}
	return 1;
}

int check_q(buffer_input *q)
{
	if(q->head==q->tail) return 0;
	if(q->head-q->tail==10||q->tail-q->head==1) return -1;
	return ((q->head-q->tail)>0)?q->head-q->tail:11+q->head-q->tail; /*number of material*/
}

void push_q(buffer_input *q,int x) /*put material into input buffer*/
{
	q->queue[(q->head)++]=x;
	genrec_buffer[2]=genrec_buffer[1];
	genrec_buffer[1]=genrec_buffer[0];
	genrec_buffer[0]=x;
	if(q->head==11) q->head=0;
	return;
}
void push_out(buffer_output *q,int x)
{
	if(q->head-q->tail==10) q->tail=1;
	if(q->tail-q->head==1) {q->tail++; if(q->tail==11) q->tail=0;}

	q->queue[(q->head)++]=x;
	genrec_buffer[2]=genrec_buffer[1];
	genrec_buffer[1]=genrec_buffer[0];
	genrec_buffer[0]=x;
	if(q->head==11) q->head=0;
	return;
}
int get_q(buffer_input *q) /*get material from input buffer*/
{
	int i;
	i=q->queue[(q->tail)++];
	if(q->tail==11) q->tail=0;
	return i;

}

int check_outp(int x,int y) /*check whether can produce this product*/
{
	/* material 1,2->0; 1,3->1; 2,3->2*/
	int i,j;
	i=(x>y)?x:y;
	j=x+y-i;
	if(i==2 && j==1) return can_produce[0];
	if(i==3 && j==1) return can_produce[1];
	if(i==3 && j==2) return can_produce[2];
}
void outp(int x,int y) /*produce this product*/
{
	/* material 1,2->0; 1,3->1; 2,3->2*/
	int i,j,a1,a2,a3;
	i=(x>y)?x:y;
	j=x+y-i;
	can_produce[0]=1;
	can_produce[1]=1;
	can_produce[2]=1;
	if(i==2 && j==1) 
	{
		push_out(oqueue,1);
		obuffer[0]++; 
		can_produce[0]=0; /*can not produce the same product next*/
	} else if(i==3 && j==1) 
	{
		push_out(oqueue,2);
		obuffer[1]++; 
		can_produce[1]=0;
	}else
	{
		push_out(oqueue,3);
		obuffer[2]++;
		can_produce[2]=0;
	}

	/*check if difference less than 10*/

	/*sort: a1-max; a2-second max; a3:min*/
	if(obuffer[0]>=obuffer[1] && obuffer[0]>=obuffer[2])
	{
		a1=0;
		if(obuffer[1]>obuffer[2]){a2=1;a3=2;} else {a2=2;a3=1;}
	}else if(obuffer[1]>=obuffer[2])
	{
		a1=1;
		if(obuffer[0]>=obuffer[2]) {a2=0;a3=2;} else{a2=2;a3=0;}
	}else
	{
		a1=2;
		if(obuffer[0]>=obuffer[1]) {a2=0;a3=1;} else{a2=1;a3=0;}
	}
	if(obuffer[a1]-obuffer[a3]>=9) can_produce[a1]=0;
	if(obuffer[a2]-obuffer[a3]>=9) can_produce[a2]=0;
}








/*begin generator*/
void* generator1(void *ptr) {
	while(1)
	{
		pthread_mutex_lock(&input_mutex);
		while(check_q(ibuffer)==-1 || !check_cangen(1)) {
		if(check_q(ibuffer)==-1) pthread_cond_wait(&condp,&input_mutex); else {
		pthread_cond_signal(&condp);
		pthread_cond_signal(&condp0); 
		pthread_cond_wait(&condp0,&input_mutex);
		}
		
		}
		push_q(ibuffer,1);
		num_material[1]++;
		num_material_used[1]++;
		pthread_cond_signal(&condc);
		pthread_cond_signal(&condp0);
		pthread_mutex_unlock(&input_mutex);
		while(pause_signal)  usleep(100);
	}
	pthread_exit(0);
}
void* generator2(void *ptr) {
	while(1)
	{
		pthread_mutex_lock(&input_mutex);
		while(check_q(ibuffer)==-1 || !check_cangen(2)) {
		if(check_q(ibuffer)==-1) pthread_cond_wait(&condp,&input_mutex); else {
		pthread_cond_signal(&condp);
		pthread_cond_signal(&condp0); 
		pthread_cond_wait(&condp0,&input_mutex);
		}
		
		}	
		push_q(ibuffer,2);
		num_material[2]++;
		num_material_used[2]++;
		pthread_cond_signal(&condc);
		pthread_cond_signal(&condp0);
		pthread_mutex_unlock(&input_mutex);
		while(pause_signal) usleep(100);
	}
	pthread_exit(0);
}
void* generator3(void *ptr) {
	while(1)
	{
		pthread_mutex_lock(&input_mutex);
		while(check_q(ibuffer)==-1 || !check_cangen(3)) {
			if(check_q(ibuffer)==-1) pthread_cond_wait(&condp,&input_mutex); else {
			pthread_cond_signal(&condp);
			pthread_cond_signal(&condp0); 
			pthread_cond_wait(&condp0,&input_mutex);
			}
			
		}
		push_q(ibuffer,3);
		num_material[3]++;
		num_material_used[3]++;
		pthread_cond_signal(&condc);
		pthread_cond_signal(&condp0);
		pthread_mutex_unlock(&input_mutex);
		while(pause_signal) usleep(100);
	}
	pthread_exit(0);
}
void* operator(void *ptr)
{
	int x,y,flag;
	while(1)
	{
		flag=0;
		pthread_mutex_lock(&tool_mutex);
		while (num_tool<2) pthread_cond_wait(&condt,&tool_mutex);
		num_tool--; /*get one tool*/
		pthread_mutex_unlock(&tool_mutex);
		pthread_mutex_lock(&tool_mutex);
		if(num_tool==0) 
		{
			num_tool++;/*put back*/
			pthread_mutex_unlock(&tool_mutex);
			continue;
		}
		num_tool--;/*get another tool*/
		pthread_mutex_unlock(&tool_mutex);
		pthread_mutex_lock(&input_mutex);
		while (check_q(ibuffer)==0) pthread_cond_wait(&condc,&input_mutex);
		x=get_q(ibuffer);/*get one material*/
		pthread_cond_signal(&condp);
		pthread_mutex_unlock(&input_mutex);
		pthread_mutex_lock(&input_mutex);
		while (check_q(ibuffer)==0) pthread_cond_wait(&condc,&input_mutex);
		y=get_q(ibuffer);/*get another material*/
		pthread_cond_signal(&condp);
		pthread_mutex_unlock(&input_mutex);
		while (x==y) /*same material or same product as before, change one!*/
		{
			pthread_mutex_lock(&input_mutex);
			while (check_q(ibuffer)==0) pthread_cond_wait(&condc,&input_mutex);
			y=get_q(ibuffer);
			push_q(ibuffer,x);
			pthread_mutex_unlock(&input_mutex);
		}
		
		
		/*Now get all things and begin to produce product*/
		usleep(1000*(10+rand()%991));/* need 0.01s to 1s to produce*/
		/*put tool back*/
		pthread_mutex_lock(&tool_mutex);
		num_tool+=2;
		pthread_cond_signal(&condt);
		pthread_mutex_unlock(&tool_mutex);
		pthread_mutex_lock(&output_mutex);
		/*wait till the product can be put into the output queue*/
		if(!check_outp(x,y)) {
			pthread_mutex_lock(&wait_mutex);
			num_wait++;
			if(num_wait==num_op) /*deadlock*/
			{
				num_deadlock++;
				num_wait--;
				pthread_mutex_lock(&input_mutex);
				/*abandon this product*/
				num_material_used[x]--;
				num_material_used[y]--;
				pthread_mutex_unlock(&input_mutex);
				pthread_mutex_unlock(&wait_mutex);
				pthread_mutex_unlock(&output_mutex);
				continue; /*drop the product*/
			}
			pthread_mutex_unlock(&wait_mutex);
			flag=1;
		}
		while(!check_outp(x,y)) pthread_cond_wait(&condc0,&output_mutex);
		pthread_mutex_lock(&wait_mutex);
		outp(x,y); /*output*/
		pthread_cond_signal(&condc0);
		pthread_mutex_unlock(&output_mutex);
		if(flag) num_wait--;/*decrease wait number*/
		pthread_mutex_unlock(&wait_mutex);
		while(pause_signal) usleep(100);
	
	}
	pthread_exit(0);
}


/*dynamic output*/
void* dynamic_output(void* ptr)
{
	int i;
	while(1){
	while(pause_signal) usleep(100);
	pthread_mutex_lock(&input_mutex); /*get buffer*/
	printf("\033[2J\033[1;1H");
	printf("Generator status:\n");
	printf("Generated: material1: %d, material2: %d, material3: %d\n",num_material[1],num_material[2],num_material[3]);
	i=check_q(ibuffer);
	if(i==-1) i=10;
	printf("Input buffer size: %d\n",i);
	printf("Input buffer: GENERATOR-> ");
	i=ibuffer->head;
	while (i!=ibuffer->tail)
	{
		i--;
		if(i==-1) i=10;
		printf("%d ",ibuffer->queue[i]);
	}
	pthread_mutex_unlock(&input_mutex);
	printf(" ->OPERATOR\n\n\n");
	printf("Operator status:\n");
	printf("Produce: product1: %d, product2: %d, product3: %d\n",obuffer[0],obuffer[1],obuffer[2]);
	pthread_mutex_lock(&output_mutex);
	printf("First ten of output buffer:");
	i=oqueue->head;
	while (i!=oqueue->tail)
	{
		i--;
		if(i==-1) i=10;
		printf("%d ",oqueue->queue[i]);
	}
	pthread_mutex_unlock(&output_mutex);
	printf("\nTools available now: %d\n",num_tool);
	printf("%d processes can\'t output the product now and are waiting\n", num_wait);
	printf("Deadlock happens %d times (Solved by drop products)",num_deadlock);
	printf("\n\nPlease notice that number of material generated maybe a little bigger than number of material used and in the buffer since some product is processing or pending to put to the output buffer\n");
	printf("\n\n\nPress  key \'CTRL + Z\' to pause the program (I deal with this signal with my own function)\nPress  key \'CTRL + C\' to terminate this program.(I deal with this signal with my own function)\n");
	printf("Please run this program in full-screen to see all information\n");
	if(obuffer[0]+obuffer[1]+obuffer[2]>MAXOUTPUT) 
	{
		printf("output buffer is too large and please restart this program. (larger than %d)\n",MAXOUTPUT);
		exit(0);
	}
	
	sleep(1);
	}
	pthread_exit(0);
}

void pause_handler(int sig)
{
	if(sig==SIGTSTP)
	{
		pause_signal=1-pause_signal;
		if(pause_signal==1) 	{
			printf("\n\n\n"); 
			printf("Program paused.\nTo resume, Press \'CTRL + Z\'\nTo Terminate, press \'CTRL + C\'\n");
		}
	}
}
void quit_handler(int sig)
{
	if(sig==SIGINT)
	{
			printf("\033[2J\033[1;1H"); 
			printf("Terminated\n");
			exit(0);
			
	}
}

/*main function*/
int main(void)
{
	pthread_t gen1,gen2,gen3;
	pthread_t op[11];
	pthread_t dy;
	char c;
	int i,j;
	/*clear screen*/
	printf("\033[2J\033[1;1H");
	printf("******************************************************\n");
	printf("*    Welcome to use my multithreading simulation     *\n");
	printf("*         Zeyu Zhao, ALL RIGHTS RESERVED             *\n");
	printf("*                                                    *\n");
	printf("*   PLEASE INPUT NUMBER 1 OR 2 TO START SIMULATION   *\n");
	printf("*  [1]Run as default mode(3 tools and 3 generators)  *\n");
	printf("*   [2]Input my own number of tools and generators   *\n");
	printf("******************************************************\n");
	printf("\n\nInput a number>>");

	/*initialization*/
	num_op=3;
	pthread_mutex_init(&input_mutex,NULL);
	pthread_mutex_init(&output_mutex,NULL);
	pthread_mutex_init(&wait_mutex,NULL);
	pthread_mutex_init(&tool_mutex,NULL);
	pthread_cond_init(&condc,NULL);
	pthread_cond_init(&condc0,NULL);
	pthread_cond_init(&condp,NULL);
	pthread_cond_init(&condp0,NULL);
	pthread_cond_init(&condt,NULL);
	num_material_used[1]=0;
	num_material_used[2]=0;
	num_material_used[3]=0;
	genrec_buffer[0]=0;
	genrec_buffer[1]=0;
	genrec_buffer[2]=0;
	pause_signal=0;
	num_tool=3;
	num_wait=0;
	num_deadlock=0;
	oqueue = (buffer_output *)malloc(sizeof(buffer_output));
	oqueue->tail=0;
	oqueue ->head=0;
	ibuffer = (buffer_input *)malloc(sizeof(buffer_input));
	ibuffer->tail=0;
	ibuffer->head=0;
	num_material[1]=0;
	num_material[2]=0;
	num_material[3]=0;
	obuffer[0]=0;
	obuffer[1]=0;
	obuffer[2]=0;
	can_produce[0]=1;
	can_produce[1]=1;
	can_produce[2]=1;

	/*wait for user input*/
	c=getchar();
	while(c!='1' && c!='2') {printf("\n\nInput a number between 1 and 2>>"); c=getchar();}
	if(c=='2')
	{
		printf("\033[2J\033[1;1H");
		printf("Input the tool number (2-100, other number will be treated as 3)>>");
		scanf("%d",&i);
		num_tool=(i>=2&&i<=100)?i:3;
		printf("\n Input the operator number (1-10, other number will be treated as 3)>>");
		scanf("%d",&i);
		num_op=(i>=1&&i<=10)?i:3;
	}
	/*start running*/
	signal(SIGINT, quit_handler);
	signal(SIGTSTP, pause_handler);
	pthread_create(&dy,NULL,dynamic_output,NULL);
	pthread_create(&gen1,NULL,generator1,NULL);
	pthread_create(&gen2,NULL,generator2,NULL);
	pthread_create(&gen3,NULL,generator3,NULL);
	for (i=1;i<=num_op;i++) pthread_create(&op[i],NULL,operator,NULL);

	pthread_join(gen1,NULL);
	pthread_join(gen2,NULL);
	pthread_join(gen3,NULL);
	pthread_join(dy,NULL);
	for (i=1;i<=num_op;i++) pthread_join(op[i],NULL);


	pthread_mutex_destroy(&input_mutex);
	pthread_mutex_destroy(&output_mutex);
	pthread_mutex_destroy(&tool_mutex);
	pthread_mutex_destroy(&wait_mutex);
	pthread_cond_destroy(&condp);
	pthread_cond_destroy(&condp0);
	pthread_cond_destroy(&condc);
	pthread_cond_destroy(&condc0);
	pthread_cond_destroy(&condt);
}
