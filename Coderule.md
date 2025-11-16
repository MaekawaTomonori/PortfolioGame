- Class Struct Enum 名前はUpperCamelCase
- 関数 UpperCamelCase()
- 定数 ALL_UPPER
- 引数 _lowerCamelCase
  
### Class 
- メンバ変数 lowerCamelCase_
- publicやprivateはclass と同じ階層、それ以外は階層を上げる
- headerではprivate -> publicの順で書き、private関数がある場合public関数の後に書く

### 関数内
- "{"は改行しない
- "}"のあとのelseは改行せず置く 

```c++
if (hoge == 0){

}else {

}
```

