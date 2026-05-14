#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ==========================================
// ESTRUTURAS DO SISTEMA
// ==========================================
typedef struct {
    int codigo;
    char nome[50];
    float preco;
    int estoque;
} Produto;

typedef struct {
    int totalVendas;
    float faturamento;
    int qtdDinh, qtdPix, qtdDeb, qtdCred;
    float valDinh, valPix, valDeb, valCred;
} DadosGlobais;

DadosGlobais globais = {0, 0.0f, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f};

// ==========================================
// GESTAO DE ARQUIVOS (BANCO DE DATOS)
// ==========================================
void salvarTudo(Produto estoque[], int totalP) {
    FILE *f1 = fopen("produtos.txt", "w");
    if (f1) {
        for (int i = 0; i < totalP; i++) {
            fprintf(f1, "%d;%s;%.2f;%d\n", estoque[i].codigo, estoque[i].nome, estoque[i].preco, estoque[i].estoque);
        }
        fclose(f1);
    }
    FILE *f2 = fopen("vendas.txt", "w");
    if (f2) {
        fprintf(f2, "%d;%.2f;%d;%d;%d;%d;%.2f;%.2f;%.2f;%.2f", 
                globais.totalVendas, globais.faturamento,
                globais.qtdDinh, globais.qtdPix, globais.qtdDeb, globais.qtdCred,
                globais.valDinh, globais.valPix, globais.valDeb, globais.valCred);
        fclose(f2);
    }
}

void carregarDados(Produto estoque[], int *totalP) {
    FILE *f1 = fopen("produtos.txt", "r");
    if (f1) {
        while (fscanf(f1, "%d;%[^;];%f;%d\n", &estoque[*totalP].codigo, estoque[*totalP].nome, &estoque[*totalP].preco, &estoque[*totalP].estoque) != EOF) {
            (*totalP)++;
        }
        fclose(f1);
    }
    FILE *f2 = fopen("vendas.txt", "r");
    if (f2) {
        fscanf(f2, "%d;%f;%d;%d;%d;%d;%f;%f;%f;%f", 
               &globais.totalVendas, &globais.faturamento,
               &globais.qtdDinh, &globais.qtdPix, &globais.qtdDeb, &globais.qtdCred,
               &globais.valDinh, &globais.valPix, &globais.valDeb, &globais.valCred);
        fclose(f2);
    }
}

void limparBuffer() {
    int c; while ((c = getchar()) != '\n' && c != EOF);
}

// ==========================================
// FUNCOES DE CADASTRO E MANUTENCAO
// ==========================================
void adicionarProduto(Produto estoque[], int *total) {
    printf("\n--- CADASTRAR NOVO PRODUTO ---\n");
    printf("Codigo: "); scanf("%d", &estoque[*total].codigo); limparBuffer();
    printf("Nome: "); fgets(estoque[*total].nome, 50, stdin);
    estoque[*total].nome[strcspn(estoque[*total].nome, "\n")] = 0;
    printf("Preco: "); scanf("%f", &estoque[*total].preco);
    printf("Estoque: "); scanf("%d", &estoque[*total].estoque);
    (*total)++;
    salvarTudo(estoque, *total);
    printf("\n>> Produto salvo com sucesso!\n");
}

void excluirProduto(Produto estoque[], int *total) {
    int cod, i, j, achou = -1;
    printf("\n--- EXCLUIR PRODUTO ---\n");
    printf("Digite o CODIGO para remover: "); scanf("%d", &cod);
    for (i = 0; i < *total; i++) {
        if (estoque[i].codigo == cod) { achou = i; break; }
    }
    if (achou != -1) {
        for (j = achou; j < *total - 1; j++) estoque[j] = estoque[j+1];
        (*total)--;
        salvarTudo(estoque, *total);
        printf("\n>> Produto removido!\n");
    } else printf("\n>> Codigo nao encontrado!\n");
}

void consultarEstoque(Produto estoque[], int total) {
    int i;
    printf("\n--- ESTOQUE ATUAL ---\n");
    for (i = 0; i < total; i++) {
        printf("COD: %d | %-15s | Qtd: %d | Preco: R$ %.2f\n", 
                estoque[i].codigo, estoque[i].nome, estoque[i].estoque, estoque[i].preco);
    }
    printf("\n1. Imprimir Relatorio | 0. Voltar: ");
    int op; scanf("%d", &op);
    if (op == 1) {
        FILE *imp = fopen("\\\\localhost\\termica", "w");
        if (imp) {
            fprintf(imp, "\x1b\x40\x1b\x45\x01\x1b\x61\x01");
            fprintf(imp, "RELATORIO DE ESTOQUE\n--------------------------------\n\n");
            fprintf(imp, "\x1b\x61\x00\n"); 
            fprintf(imp, " COD  PRODUTO             QTD\n");
            for (i = 0; i < total; i++) {
                fprintf(imp, " %-4d %-20.20s %3d\n", estoque[i].codigo, estoque[i].nome, estoque[i].estoque);
            }
            fprintf(imp, "--------------------------------\n\n\n");
            fclose(imp);
            printf(">> Enviado para a impressora!\n");
        }
    }
}

// ==========================================
// VENDAS E IMPRESSAO
// ==========================================
void registrarCompra(Produto estoque[], int totalP) {
    int cod, qtd, i, achou, cont = 1;
    float subTotal = 0, valorBrutoVenda = 0, totalFinal = 0, pago = 0, troco = 0, taxa = 0, juros = 0, desconto = 0;
    int itensVenda = 0, formaPgto, parcelas = 1;
    char descPgto[30], cpf[20];
    
    typedef struct { int c; char n[15]; int q; float p; float t; } ItemTemp;
    ItemTemp carrinho[100];
    int nItens = 0;

    printf("\nCPF (ou 0 para pular): "); scanf("%s", cpf);

    do {
        printf("Codigo do Produto: "); scanf("%d", &cod);
        achou = -1;
        for (i = 0; i < totalP; i++) { if (estoque[i].codigo == cod) { achou = i; break; } }
        
        if (achou != -1) {
            printf("[%s] Preco: R$ %.2f | Estoque: %d\n", estoque[achou].nome, estoque[achou].preco, estoque[achou].estoque);
            printf("Quantidade: "); scanf("%d", &qtd);
            
            if (estoque[achou].estoque >= qtd) {
                carrinho[nItens].c = estoque[achou].codigo;
                strncpy(carrinho[nItens].n, estoque[achou].nome, 14);
                carrinho[nItens].q = qtd;
                carrinho[nItens].p = estoque[achou].preco;
                carrinho[nItens].t = estoque[achou].preco * qtd;
                
                subTotal += carrinho[nItens].t;
                estoque[achou].estoque -= qtd;
                itensVenda += qtd;
                nItens++;
                printf(">> OK! Subtotal Atual: R$ %.2f\n", subTotal);
            } else printf(">> Erro: Estoque insuficiente!\n");
        } else printf(">> Erro: Codigo nao encontrado!\n");

        printf("1. Proximo Item | 0. Finalizar: "); scanf("%d", &cont);
    } while (cont == 1 && nItens < 100);

    if (subTotal > 0) {
        if (itensVenda >= 20) desconto = subTotal * 0.04;
        else if (itensVenda >= 10) desconto = subTotal * 0.02;
        
        valorBrutoVenda = subTotal - desconto;
        totalFinal = valorBrutoVenda;

        // --- ALTERADO PARA COLUNA AQUI ---
        printf("\n--- PAGAMENTO ---\n");
        printf("1. Dinheiro\n");
        printf("2. Pix\n");
        printf("3. Cartao Debito\n");
        printf("4. Cartao Credito\n");
        printf("Opcao: ");
        scanf("%d", &formaPgto);

        if (formaPgto == 1) {
            strcpy(descPgto, "DINHEIRO");
            printf("Total: R$ %.2f | Valor Pago: ", totalFinal); scanf("%f", &pago);
            troco = pago - totalFinal;
            globais.qtdDinh++; globais.valDinh += valorBrutoVenda;
        } else {
            if (formaPgto == 4) {
                taxa = totalFinal * 0.05;
                totalFinal += taxa;
                strcpy(descPgto, "CARTAO CREDITO");
                globais.qtdCred++; globais.valCred += valorBrutoVenda;
            } else if (formaPgto == 2) {
                strcpy(descPgto, "PIX");
                globais.qtdPix++; globais.valPix += valorBrutoVenda;
            } else {
                strcpy(descPgto, "CARTAO DEBITO");
                globais.qtdDeb++; globais.valDeb += valorBrutoVenda;
            }

            printf("Parcelas (1 a 12): "); scanf("%d", &parcelas);
            if (parcelas > 1) {
                if (parcelas > 12) parcelas = 12;
                juros = totalFinal * (0.02 * parcelas);
                totalFinal += juros;
            } else parcelas = 1;
            pago = totalFinal; troco = 0;
        }

        globais.totalVendas++; globais.faturamento += totalFinal;
        salvarTudo(estoque, totalP);

        FILE *imp = fopen("\\\\localhost\\termica", "w");
        if (imp) {
            time_t t = time(NULL); struct tm tm = *localtime(&t);
            fprintf(imp, "\x1b\x40\x1b\x45\x01\x1b\x61\x01"); 
            fprintf(imp, "MARKET SYSTEM\n");
            fprintf(imp, "Rua Pedro Alvares Cabral, 905\n");
            fprintf(imp, "Sao Francisco de Assis - DV\n");
            fprintf(imp, "--------------------------------\n");
            fprintf(imp, "\x1b\x61\x00"); 
            fprintf(imp, " DATA: %02d/%02d/%d %02d:%02d\n", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min);
            fprintf(imp, " CPF: %s\n", cpf);
            fprintf(imp, "--------------------------------\n");
            fprintf(imp, "COD QTD  PRODUTO    V.UNI  V.TOT\n");
            for(i=0; i<nItens; i++) {
                fprintf(imp, " %-3d %-2d %-10.10s %6.2f %6.2f\n", 
                        carrinho[i].c, carrinho[i].q, carrinho[i].n, carrinho[i].p, carrinho[i].t);
            }
            fprintf(imp, "--------------------------------\n");
            fprintf(imp, " SUBVALOR:          R$ %8.2f\n", subTotal);
            if (desconto > 0) fprintf(imp, " DESCONTO QTD:      R$ %8.2f\n", desconto);
            fprintf(imp, " FORMA PGTO: %19s\n", descPgto);
            if (taxa > 0) fprintf(imp, " TAXA CREDITO:      R$ %8.2f\n", taxa);
            if (parcelas > 1) fprintf(imp, " PARCELADO: %2dx     R$ %8.2f\n", parcelas, totalFinal/parcelas);
            if (juros > 0) fprintf(imp, " JUROS PARCELA:     R$ %8.2f\n", juros);
            fprintf(imp, " VALOR TOTAL:       R$ %8.2f\n", totalFinal);
            if (formaPgto == 1) {
                fprintf(imp, " VALOR PAGO:        R$ %8.2f\n", pago);
                fprintf(imp, " TROCO:             R$ %8.2f\n", troco);
            }
            fprintf(imp, "--------------------------------\n\n\n"); // CORRECAO DOS \n
            fclose(imp);
            printf(">> Venda concluida e cupom impresso!\n");
        }
    }
}

// ==========================================
// EXTRATO FINANCEIRO
// ==========================================
void exibirExtrato() {
    printf("\n======= FECHAMENTO DE CAIXA (TELA) =======\n");
    printf("TOTAL VENDAS: %d\n", globais.totalVendas);
    printf("FATURAMENTO TOTAL:  R$ %.2f\n", globais.faturamento);
    printf("------------------------------------------\n");
    printf("DINHEIRO: Vendas: %d | Total: R$ %.2f\n", globais.qtdDinh, globais.valDinh);
    printf("PIX:      Vendas: %d | Total: R$ %.2f\n", globais.qtdPix, globais.valPix);
    printf("DEBITO:   Vendas: %d | Total: R$ %.2f\n", globais.qtdDeb, globais.valDeb);
    printf("CREDITO:  Vendas: %d | Total: R$ %.2f\n", globais.qtdCred, globais.valCred);
    printf("==========================================\n");

    printf("\n1. Imprimir Fechamento | 0. Voltar: ");
    int op; scanf("%d", &op);
    if (op == 1) {
        FILE *imp = fopen("\\\\localhost\\termica", "w");
        if (imp) {
            fprintf(imp, "\x1b\x40\x1b\x45\x01\x1b\x61\x01");
            fprintf(imp, "FECHAMENTO DE CAIXA\n--------------------------------\n\n");
            fprintf(imp, "\x1b\x61\x00");
            fprintf(imp, " TOTAL VENDAS: %16d\n", globais.totalVendas); 
            fprintf(imp, " FATURAMENTO:  R$ %13.2f\n", globais.faturamento);
            fprintf(imp, "--------------------------------\n");
            fprintf(imp, " DINHEIRO:(%d) R$ %13.2f\n", globais.qtdDinh, globais.valDinh);
            fprintf(imp, " PIX:     (%d) R$ %13.2f\n", globais.qtdPix, globais.valPix);
            fprintf(imp, " DEBITO:  (%d) R$ %13.2f\n", globais.qtdDeb, globais.valDeb);
            fprintf(imp, " CREDITO: (%d) R$ %13.2f\n", globais.qtdCred, globais.valCred);
            fprintf(imp, "--------------------------------\n\n\n"); // CORRECAO DOS \n
            fclose(imp);
            printf(">> Extrato impresso!\n");
        }
    }
}

// ==========================================
// MENU PRINCIPAL
// ==========================================
int main() {
    Produto estoque[100];
    int totalP = 0, op;
    carregarDados(estoque, &totalP);

    do {
        printf("\n========== MARKET SYSTEM ==========\n");
        printf("1. REALIZAR VENDA\n");
        printf("2. CONSULTAR ESTOQUE\n");
        printf("3. CADASTRAR PRODUTO\n");
        printf("4. EXCLUIR PRODUTO\n");
        printf("5. EXTRATO FINANCEIRO\n");
        printf("0. SAIR\n");
        printf("-----------------------------------\n");
        printf("Opcao: ");
        
        if (scanf("%d", &op) != 1) {
            limparBuffer();
            continue;
        }

        switch(op) {
            case 1: registrarCompra(estoque, totalP); break;
            case 2: consultarEstoque(estoque, totalP); break;
            case 3: adicionarProduto(estoque, &totalP); break;
            case 4: excluirProduto(estoque, &totalP); break;
            case 5: exibirExtrato(); break;
            case 0: printf("Saindo e salvando dados...\n"); break;
            default: printf("Opcao invalida!\n");
        }
    } while (op != 0);

    return 0;
}
