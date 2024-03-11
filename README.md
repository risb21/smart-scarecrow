# smart-scarecrow
Creating a smart scarecrow system to detect birds using computer vision and scare them away using directional speakers


## Steps to install and use esp-idf 

Navigate to the `esp-idf` directory in `lib`, then install the framework

```bash
cd lib/esp-idf
sh install.sh
```

Add the export shell script to `.bashrc` if you use bash, or `.zshrc/zprofile` etc. if you use zsh

```bash
vi ~/.bashrc
```

Paste the following path to the export script in the file

```bash
export IDF_TOOLS_EXPORT_CMD="<repository location>/lib/esp-idf/export.sh"
```

<b>Upon opening a new terminal, to use the framework you must source the export script every time.</b>

```bash
source $IDF_TOOLS_EXPORT_CMD
```

