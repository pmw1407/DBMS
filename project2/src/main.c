#include "bpt.h"
#include "file.h"

// MAIN

extern FILE * fp;
extern page_t* current_page;

int main( int argc, char ** argv ) {

    int table_id;
    char input_file[100];
    int input, range2;
    int instruction;
    char license_part;

    int64_t key;
    char value[120];
    
    verbose_output = false;

    //license_notice();
    //usage_1();  
    //usage_2();

    while(true){
        show_menu();
        scanf("%d", &instruction);

        if(instruction == 5)
            break;
        
        else if(instruction == 1){
            printf("Input file name : ");
            scanf("%s", input_file);
            open_table(input_file);
        }

        else if(instruction == 2){
            printf("Input key : ");
            scanf("%ld", &key);
            printf("Input value : ");
            scanf("%s", value);

            db_insert(key, value);
            print_tree();
        }

        else if(instruction == 4){
            printf("Input key : ");
            scanf("%ld", &key);

            db_delete(key);

            print_tree();
        }

        else if(instruction == 6){
            print_tree();
        }

        else if(instruction == 7){
            printf("Insert Range start from 1 : ");
            int64_t end;
            scanf("%ld", &end);
            char a[120];
            strcpy(a, "a");
            for(int i = 1; i <= end; i++){
                db_insert(i, a);
            }
        }

        else if(instruction == 8){
            printf("Delete Range start from 1 : ");
            int64_t end;
            scanf("%ld", &end);
            for(int i = 1; i <= end; i++){
                db_delete(i);
            }
        }

        else{
            printf("Wrong instruction input. Please try again\n");
            getchar();
            
        }
    }

    fclose(fp);
    
    return EXIT_SUCCESS;
    
}
