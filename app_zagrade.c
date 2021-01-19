#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define SIZE 50
static char str[50];


int s[SIZE];
int top = -1;
int top1 = -1;
int flag1 = 0;

int priority (char c)
{
  switch(c)
  {
      case '+':return 1;
      case '-':return 2;
      case '*':return 3;
      case '/':return 4;
  }
  return 0;
}
void push(char c)
{
   str[++top]=c;
}
char pop()
{
   return (str[top--]);
}


FILE* fp;

const char* read_alu()
{
	char *str = NULL;
	size_t len = 0;
	
	fp = fopen("/dev/alu", "r");
	
	// str = (char *) malloc(result_bytes + 1); // +1 zbog nove linije
	getline(&str, &len, fp);
	
	
	
	if(fclose(fp)) {
		printf("ERROR ON read_alu()");
	}

	return str;
}

int write_format(char *format) {
	fp = fopen("/dev/alu", "w");
	
	fprintf(fp, "format=%s\n", format);
	
	if(fclose(fp)) {
		printf("ERROR ON write_format()");
		return -1;
	}
}

int write_reg(char *reg, int number) {
	fp = fopen("/dev/alu", "w");
	
	fprintf(fp, "%s=0x%x\n", reg, number);
	
	if(fclose(fp)) {
		printf("ERROR ON write_reg()");
		return -1;
	}
}

int write_alu(char *reg_one, char *reg_two, char *operation) {
	fp = fopen("/dev/alu", "w");

	fprintf(fp,"%s %s %s\n", reg_one, operation, reg_two);
	
	if(fclose(fp)) {
		printf("ERROR ON write_alu()");
		return -1;
	}
}
double pop1()
{
  return (s[top1--]);
}


void push1(int elem)
{
    if(flag1==1)
    {
        int num;
        num = pop1();
        char str_broj1[20], str_broj2[20];
        write_reg("regA", 10);
        write_reg("regB", num);
        write_alu("regA", "regB", "*");
        sscanf(read_alu(), "%4s 0", str_broj1);
        int broj1 = atoi(str_broj1);
        write_reg("regA", broj1);
        write_reg("regB", elem);
        write_alu("regA", "regB", "+");
        sscanf(read_alu(), "%4s 0", str_broj2);
        int broj2 = atoi(str_broj2);
        s[++top1] = broj2;
    }
    else if(flag1==0)
    {
        s[++top1]=elem;
        flag1=1;
    }
}

int main()
{
	write_format("app");
    char in[50],post[50],ch;
    int i,j,l;
    int izlaz;
    int br_rez;
    int prekoracenje;
    char str_rez[50];
    do
    {
		printf("\n Unesite string: ");
        scanf("%s", in);
        l=strlen(in);
        izlaz = strcmp(in, "exit");
   
    
    
    
	
	
			for(i=0,j=0;i<l;i++)
			{

				if (isalpha(in[i])||isdigit(in[i]))
				    post[j++]=in[i];
				else
				{
				     post[j++] = ' ';
				     if(in[i]=='(')
				        push(in[i]);
				     else if(in[i]==')')
				        while((ch=pop())!='(')
				            post[j++]=ch;
				     else
				     {
				         while(priority(in[i])<=priority(str[top]))
				         {
				            post[j++]=pop();
				            post[j++]= ' ';
				         }
				         push(in[i]);
				     }

				}
			}
			while(top!=-1)
			{
				post[j++] = ' ';
				post[j++]=pop();
			}
			post[j]='\0';
		//	printf("\n Postfiksna forma:%s\n ",post);



			i = 0;
			double  op1, op2;
			int duzina = strlen(post);
			while( i < duzina)
			{
				ch = post[i];
				if(isdigit(ch))
				{
				    push1(ch-'0');
				}
				else if(ch ==' ')
				{
				    flag1=0;
				}
				else
				{
				    flag1=0;
				    op2=pop1();
				    op1=pop1();
				    write_reg("regC", op1);
    				    write_reg("regD", op2);
				    switch(ch)
				    {
				        case '+':write_alu("regC", "regD", "+");
    					  	  sscanf(read_alu(), "%4s %d", str_rez, &prekoracenje);
        					  br_rez = atoi(str_rez);
				        	  push1(br_rez); 
				        	  break;
				        case '-':write_alu("regC", "regD", "-");
    						  sscanf(read_alu(), "%4s %d", str_rez, &prekoracenje);
        					  br_rez = atoi(str_rez);
				                 push1(br_rez);
				                 break;
				        case '*':write_alu("regC", "regD", "*");
    						  sscanf(read_alu(), "%4s %d", str_rez, &prekoracenje);
        					  br_rez = atoi(str_rez);
				        	  push1(br_rez);
				                 break;
				        case '/':write_alu("regC", "regD", "/");
    						  sscanf(read_alu(), "%4s %d", str_rez, &prekoracenje);
        					  br_rez = atoi(str_rez);
				        	  push1(br_rez);
				                 break;

				    }
				}
				i++;
				if (prekoracenje == 1)
				{  	
					strcpy(in, "exit");
				    	printf("Doslo je do prekoracenja opsega\n");
				 }
			}
			izlaz = strcmp(in, "exit");
			if (izlaz != 0)
			{
				printf("Rezultat: %d\n",s[top1]);
				write_reg("regA", 0);
				write_reg("regB", 0);
				write_reg("regC", 0);
				write_reg("regD", 0);
				strcpy(post, " ");
				for (int br = 0; br < SIZE; br++)
					s[br] = 0;
				top = -1;
				top1 = -1;
				flag1 = 0;
				
			}
			//else 
				//printf("EXIT");
			
		  
    }
    while (izlaz);
    write_reg("regA", 0);
    write_reg("regB", 0);
    write_reg("regC", 0);
    write_reg("regD", 0);
    write_alu("regA", "regB", "+");
    write_format("hex");
    return 0;
 }
