#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define SIZE 50
static char str[30];


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

const char* read_alu() {
	char *str = NULL;
	size_t len = 0;
	// jel si kopirala ove funkcije potupuno?da
	fp = fopen("/dev/alu", "r");
	
	// str = (char *) malloc(len + 1); // +1 zbog nove linije
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


int ispravan_izraz(char *izraz)
{
    int operand[] = {0, 0, 0, 0};
    int i = 0, j, br_operanada = 0, br_operacija = 0;
    char izraz_char;
    int izraz_int;
    char pozicija_operacije[5], operacija[5];

    if (strcmp(izraz, "exit") == 0)
    	return 0;

    if (izraz[0] == '*' || izraz[0] == '/') //proverava dal izraz pocinje za znakom * ili /
    {
        printf("Neispravan unos izraza\n");
        return 0;
    }

    int duzina = strlen(izraz);
    if (izraz[duzina - 1] == '+' || izraz[duzina - 1] == '-' || izraz[duzina - 1] == '*' || izraz[duzina - 1] == '/') // provera dal se izraz ne zavrsava nekom od operacija
    {
        printf("Neispravan unos izraza\n");
        return 0;
    }

    if (izraz[0] == '*' || izraz[0] == '/') //proverava dal izraz pocinje za znakom * ili /
    {
        printf("Neispravan unos izraza\n");
        return 0;
    }

    while (izraz[i] != '\0')
    {
        izraz_char = izraz[i];
        if (izraz_char != '+' && izraz_char != '-' && izraz_char != '*' && izraz_char != '/')
        {
            izraz_int = izraz_char - '0'; //izdvajanje cifre
            operand[br_operanada] = operand[br_operanada] * 10 + izraz_int; //pravljenje operanada od cifara
        }
        else
        {
            pozicija_operacije[br_operacija] = i; //zapisujemo poziciju svake operacije
            operacija[br_operacija] = izraz[i]; // zapisujemo operaciju
            br_operacija++;
            br_operanada = br_operanada + (i != 0); // ne uvecavamo broj operanada ako je prvi znak u izrazu operacija
        }
        i++;
    }
    br_operanada++; // ukupan broj operanada


    for (i = 0; i < br_operacija; i++)
    {
        if (pozicija_operacije[i] + 1 == pozicija_operacije[i + 1]) // ako su na dve susedne pozicije operacije, to predstavlja neispravno unet izraz
        {
            printf("Neispravan unos operacija\n");
            return 0;
        }
        if (br_operanada > 4) // ukoliko je broj operanada veci od 4, to predstavlja neispravno unet izraz
        {
            printf("Prekoracen unos operanada\n");
            return 0;

        }
    }

    for (i = 0; i < br_operanada; i++)
    {
        if (operand[i] > 255) // ako je vrednost nekog operanda veca od 255, to predstavlja nepravilno unet izraz
        {
            printf("Prekoracenje vrednosti operanda\n");
            return 0;
        }

    }

   /* printf("Ispravno unet izraz\n");
    for (i = 0; i < br_operanada; i++) //ispisujemo svaki operand
        printf("Operand%d = %d\n", i + 1, operand[i]);


    for (i = 0; i < br_operacija; i++) //ispisujemo svaku operaciju
       printf("Operacija%d = %c\n", i, operacija[i]);*/
    return 1;
}








int main()



{	
	write_format("dec");
    char in[30],post[30],ch;
    int i,j,l;
    int izlaz;


    do
    {

        printf("Unesite matematicki izraz sa maksimalno 4 celobrojna operanda u opsegu [0-255]\n" );
        scanf("%s", in);
        if (ispravan_izraz(in))
        {
        	l=strlen(in);
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
			//printf("\n Postfiksna forma:%s\n ",post);



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
				    switch(ch)
				    {
				        case '+':push1(op1+op2); 
				                 break;
				        case '-':push1(op1-op2);
				                 break;
				        case '*':push1(op1*op2);
				                 break;
				        case '/':push1(op1/op2);
				                 break;

				    }
				}
				i++;
				
			}
			
    	}
    	else 
    		strcpy(in, "exit");
    	izlaz = strcmp(in, "exit");
		if (izlaz != 0)
		{
				printf("Rezultat: %d\n",s[top1]);
				write_reg("regA", 0);
				write_reg("regB", 0);
				strcpy(post, " ");
				for (int br = 0; br < SIZE; br++)
					s[br] = 0;
				top = -1;
				top1 = -1;
				flag1 = 0;
				
		}
		else 
				printf("EXIT\n");
    }
    while (izlaz);
    return 0;
    
    
 }
