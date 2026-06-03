#include <stdio.h>

int main(void)
{
    char buf[1024];
    printf("请输入任意带空格的整行文字：");
    fgets(buf, sizeof(buf), stdin); // 读取一整行，包含空格
    
    printf("输出内容：%s", buf);
    return 0;
}
