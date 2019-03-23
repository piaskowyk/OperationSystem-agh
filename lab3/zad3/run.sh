#/bin/bash
make clean_all
make
make clean

mkdir dane

numOfBlock=([1]=9999 [4]=9999 [512]=9999 [1024]=9999 [4096]=9999 [8192]=9999)

for index in 1 4 512 1024 4096 8192
do
    echo "Generate random file: number of block - ${numOfBlock[$index]}, blck size - $index"
    echo "Generate random file: number of block - ${numOfBlock[$index]}, blck size - $index" >> raport1.txt
    ./main generate ./dane/dane$index ${numOfBlock[$index]} $index >> raport1.txt
    echo ""
    echo "" >> raport1.txt

    echo "Copy file dane to daneSys$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - sys"
    echo "Copy file dane to daneSys$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - sys" >> raport1.txt
    ./main copy ./dane/dane$index ./dane/daneSys$index ${numOfBlock[$index]} $index sys >> raport1.txt
    echo ""
    echo "" >> raport1.txt

    echo "Copy file dane to daneLib$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - lib"
    echo "Copy file dane to daneLib$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - lib" >> raport1.txt
    ./main copy ./dane/dane$index ./dane/daneLib$index ${numOfBlock[$index]} $index lib >> raport1.txt
    echo ""
    echo "" >> raport1.txt

    echo "Sort file daneSys$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - sys"
    echo "Sort file daneSys$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - sys" >> raport1.txt
    ./main sort ./dane/daneSys$index ${numOfBlock[$index]} $index sys >> raport1.txt
    echo ""
    echo "" >> raport1.txt

    echo "Sort file daneLib$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - lib"
    echo "Sort file daneLib$index: number of block - ${numOfBlock[$index]}, blck size - $index, type - lib" >> raport1.txt
    ./main sort ./dane/daneLib$index ${numOfBlock[$index]} $index lib >> raport1.txt

    echo ""
    echo "" >> raport1.txt
    echo "-------------------------------------------------------------"
    echo "-------------------------------------------------------------" >> raport1.txt
    echo ""
    echo "" >> raport1.txt
done

echo "W pliku raport1.txt znajdują się dane z pomiarów czasowych dla każdej operacji."


echo "" >> raport1.txt
echo "" >> raport1.txt
echo "Wnioski:" >> raport1.txt
echo "Dla małych rozmiarów bloków, szybsze są funkcje biblioteczne natomiast wraz z zwiększaniem się rozmiarów bloków szybsze stają się funkcje systemowe." >> raport1.txt
