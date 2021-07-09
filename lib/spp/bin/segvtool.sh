#!/bin/sh
# This script was generated using Makeself 2.1.2
CRCsum="553934194"
MD5="7da19d3bfc524d2b98884e2b61a1c503"
TMPROOT=${TMPDIR:=/tmp}

label="SegvTool"
script="./segvtool.sh"
scriptargs="`pwd`"
targetdir="segvtool"
filesizes="24218"
keep=n

print_cmd_arg=""
if type printf > /dev/null; then
    print_cmd="printf"
elif test -x /usr/ucb/echo; then
    print_cmd="/usr/ucb/echo"
else
    print_cmd="echo"
fi

MS_Printf()
{
    $print_cmd $print_cmd_arg "$1"
}

MS_Progress()
{
    while read a; do
	MS_Printf .
    done
}

MS_dd()
{
    if [ "$(which tdds 2>/dev/null)" ]
    then 
        DD_BIN="tdds"
    elif [ "$(which dd 2>/dev/null)" ] 
    then
        DD_BIN="dd"
    else
        echo "ERROR: can not find dd or tdds,exit...."
        exit 1
    fi

    blocks=`expr $3 / 1024`
    bytes=`expr $3 % 1024`
    $DD_BIN if="$1" ibs=$2 skip=1 obs=1024 conv=sync 2> /dev/null | \
    { test $blocks -gt 0 && $DD_BIN ibs=1024 obs=1024 count=$blocks ; \
      test $bytes  -gt 0 && $DD_BIN ibs=1 obs=1024 count=$bytes ; } 2> /dev/null
}

MS_Help()
{
    cat << EOH >&2
Makeself version 2.1.2
 1) Getting help or info about $0 :
  $0 --help   Print this message
  $0 --info   Print embedded info : title, default target directory, embedded script ...
  $0 --lsm    Print embedded lsm entry (or no LSM)
  $0 --list   Print the list of files in the archive
  $0 --check  Checks integrity of the archive
 
 2) Running $0 :
  $0 [options] [--] [additional arguments to embedded script]
  with following options (in that order)
  --confirm             Ask before running embedded script
  --keep                Do not erase target directory after running
			the embedded script
  --nox11               Do not spawn an xterm
  --nochown             Do not give the extracted files to the current user
  --target NewDirectory Extract in NewDirectory
  --                    Following arguments will be passed to the embedded script
EOH
}

MS_Check()
{
    OLD_PATH=$PATH
    PATH=${GUESS_MD5_PATH:-"$OLD_PATH:/bin:/usr/bin:/sbin:/usr/local/ssl/bin:/usr/local/bin:/opt/openssl/bin"}
    MD5_PATH=`which md5sum 2>/dev/null || type md5sum 2>/dev/null`
    MD5_PATH=${MD5_PATH:-`which md5 2>/dev/null || type md5 2>/dev/null`}
    PATH=$OLD_PATH
    MS_Printf "Verifying archive integrity..."
    offset=`head -n 361 "$1" | wc -c | tr -d " "`
    verb=$2
    i=1
    for s in $filesizes
    do
	crc=`echo $CRCsum | cut -d" " -f$i`
	if test -x "$MD5_PATH"; then
	    md5=`echo $MD5 | cut -d" " -f$i`
	    if test $md5 = "00000000000000000000000000000000"; then
		test x$verb = xy && echo " $1 does not contain an embedded MD5 checksum." >&2
	    else
		md5sum=`MS_dd "$1" $offset $s | "$MD5_PATH" | cut -b-32`;
		if test "$md5sum" != "$md5"; then
		    echo "Error in MD5 checksums: $md5sum is different from $md5" >&2
		    exit 2
		else
		    test x$verb = xy && MS_Printf " MD5 checksums are OK." >&2
		fi
		crc="0000000000"; verb=n
	    fi
	fi
	if test $crc = "0000000000"; then
	    test x$verb = xy && echo " $1 does not contain a CRC checksum." >&2
	else
	    sum1=`MS_dd "$1" $offset $s | cksum | awk '{print $1}'`
	    if test "$sum1" = "$crc"; then
		test x$verb = xy && MS_Printf " CRC checksums are OK." >&2
	    else
		echo "Error in checksums: $sum1 is different from $crc"
		exit 2;
	    fi
	fi
	i=`expr $i + 1`
	offset=`expr $offset + $s`
    done
    echo " All good."
}

UnTAR()
{
    if [ "$(which tar 2>/dev/null)" ]
    then 
        TAR_BIN="tar"
    elif [ "$(which ttars 2>/dev/null)" ] 
    then
        TAR_BIN="ttars"
    else
        echo "ERROR: can not find tar or ttars,exit...."
        exit 1
    fi
    $TAR_BIN $1vf - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 $$; }
}

finish=true
xterm_loop=
nox11=n
copy=none
ownership=

while true
do
    case "$1" in
    -h | --help)
	MS_Help
	exit 0
	;;
    --info)
	echo Identification: "$label"
	echo Target directory: "$targetdir"
	echo Uncompressed size: 96 KB
	echo Compression: gzip
	echo Date of packaging: Thu Mar 20 12:59:55 CST 2014
	echo Built with Makeself version 2.1.2 on linux
	if test x$script != x; then
	    echo Script run after extraction:
	    echo "    " $script $scriptargs
	fi
	if test x"" = xcopy; then
		echo "Archive will copy itself to a temporary location"
	fi
	if test x"n" = xy; then
	    echo "directory $targetdir is permanent"
	else
	    echo "$targetdir will be removed after extraction"
	fi
	exit 0
	;;
    --dumpconf)
	echo LABEL=\"$label\"
	echo SCRIPT=\"$script\"
	echo SCRIPTARGS=\"$scriptargs\"
	echo archdirname=\"segvtool\"
	echo KEEP=n
	echo COMPRESS=gzip
	echo filesizes=\"$filesizes\"
	echo CRCsum=\"$CRCsum\"
	echo MD5sum=\"$MD5\"
	echo OLDUSIZE=96
	echo OLDSKIP=362
	exit 0
	;;
    --lsm)
cat << EOLSM
No LSM.
EOLSM
	exit 0
	;;
    --list)
	echo Target directory: $targetdir
	offset=`head -n 361 "$0" | wc -c | tr -d " "`
	for s in $filesizes
	do
	    MS_dd "$0" $offset $s | eval "gzip -cd" | UnTAR t
	    offset=`expr $offset + $s`
	done
	exit 0
	;;
    --check)
	MS_Check "$0" y
	exit 0
	;;
    --confirm)
	verbose=y
	shift
	;;
    --keep)
	keep=y
	shift
	;;
    --target)
	keep=y
	targetdir=${2:-.}
	shift 2
	;;
    --nox11)
	nox11=y
	shift
	;;
    --nochown)
	ownership=n
	shift
	;;
    --xwin)
	finish="echo Press Return to close this window...; read junk"
	xterm_loop=1
	shift
	;;
    --phase2)
	copy=phase2
	shift
	;;
    --)
	shift
	break ;;
    -*)
	echo Unrecognized flag : "$1" >&2
	MS_Help
	exit 1
	;;
    *)
	break ;;
    esac
done

case "$copy" in
copy)
    SCRIPT_COPY="$TMPROOT/makeself$$"
    echo "Copying to a temporary location..." >&2
    cp "$0" "$SCRIPT_COPY"
    chmod +x "$SCRIPT_COPY"
    cd "$TMPROOT"
    exec "$SCRIPT_COPY" --phase2
    ;;
phase2)
    finish="$finish ; rm -f $0"
    ;;
esac

if test "$nox11" = "n"; then
    if tty -s; then                 # Do we have a terminal?
	:
    else
        if test x"$DISPLAY" != x -a x"$xterm_loop" = x; then  # No, but do we have X?
            if xset q > /dev/null 2>&1; then # Check for valid DISPLAY variable
                GUESS_XTERMS="xterm rxvt dtterm eterm Eterm kvt konsole aterm"
                for a in $GUESS_XTERMS; do
                    if type $a >/dev/null 2>&1; then
                        XTERM=$a
                        break
                    fi
                done
                chmod a+x $0 || echo Please add execution rights on $0
                if test `echo "$0" | cut -c1` = "/"; then # Spawn a terminal!
                    exec $XTERM -title "$label" -e "$0" --xwin "$@"
                else
                    exec $XTERM -title "$label" -e "./$0" --xwin "$@"
                fi
            fi
        fi
    fi
fi

if test "$targetdir" = "."; then
    tmpdir="."
else
    if test "$keep" = y; then
	echo "Creating directory $targetdir" >&2
	tmpdir="$targetdir"
    else
	tmpdir="$TMPROOT/selfgz$$"
    fi
    mkdir $tmpdir || {
	echo 'Cannot create target directory' $tmpdir >&2
	echo 'You should try option --target OtherDirectory' >&2
	eval $finish
	exit 1
    }
fi

location="`pwd`"
if test x$SETUP_NOCHECK != x1; then
    MS_Check "$0"
fi
offset=`head -n 361 "$0" | wc -c | tr -d " "`

if test x"$verbose" = xy; then
	MS_Printf "About to extract 96 KB in $tmpdir ... Proceed ? [Y/n] "
	read yn
	if test x"$yn" = xn; then
		eval $finish; exit 1
	fi
fi

MS_Printf "Uncompressing $label"
res=3
if test "$keep" = n; then
    trap 'echo Signal caught, cleaning up >&2; cd $TMPROOT; /bin/rm -rf $tmpdir; eval $finish; exit 15' 1 2 3 15
fi

for s in $filesizes
do
    if MS_dd "$0" $offset $s | eval "gzip -cd" | ( cd "$tmpdir"; UnTAR x ) | MS_Progress; then
	if test x"$ownership" != x; then
	    (PATH=/usr/xpg4/bin:$PATH; cd "$tmpdir"; chown -R `id -u` .;  chgrp -R `id -g` .)
	fi
    else
	echo
	echo "Unable to decompress $0" >&2
	eval $finish; exit 1
    fi
    offset=`expr $offset + $s`
done
echo

cd "$tmpdir"
res=0
if test x"$script" != x; then
    if test x"$verbose" = xy; then
	MS_Printf "OK to execute: $script $scriptargs $* ? [Y/n] "
	read yn
	if test x"$yn" = x -o x"$yn" = xy -o x"$yn" = xY; then
	    $script $scriptargs $*; res=$?;
	fi
    else
	$script $scriptargs $*; res=$?
    fi
    if test $res -ne 0; then
	test x"$verbose" = xy && echo "The program '$script' returned an error code ($res)" >&2
    fi
fi
if test "$keep" = n; then
    cd $TMPROOT
    /bin/rm -rf $tmpdir
fi
eval $finish; exit $res
� �u*S�||U����B�J"(ڵU�A$��V�CS��A���n�ҕG���!R��Y���㮏U?����mR
�E�R䡸PeBxT�P���9��$��
�������t�{�9�s���;wf�sN�93�̛�����7��>|8�¿��z����9됡����}���`���si�����E�#��Ҹ���I�T�R����{*�qO��N�qwq����,��R��+"<Y�H��M��@:AW.�ˊ������Q-?~#�Xzd\V�5E-�]�kB�YQ��\ԕ��a>�ڪ�[+r��#yFo����-gԴ�Q�
XE�}�����7A�K�����t��|��'��k�V1|��I3�~�,=㉬��jA���|]:�^4�n�]��n�=���s�n]0rĭ#�*�7h�mV}>�މQ~�����x�����1t��
~��~��^���_�]����w�~7��K�����tm����?���;��F���k�}��Wm�I�~�¯���{^OOCt���k0��-����ߨ^��ПU˗��O��Ծ~\��Q���о�;��P]��(���z�0����gY]�ƅ1�� �^�V����^�������1t����}��$����?;��>�~�zm�|E��z���]1�~��_j��j�k�aCL��s\�v����PC_��w1���N�ԁ]�;(�M[<큙�I3�g�~��hf7mڣs�͝�3�4mCW�T#��Ys��\����s�|(X$͗�͞����>Y3�<:S�/�7_�����8���
r�3�ϟ7�+tBF��y�\~��yE3�|Ub�T0g&�̐�/ʚW(̝��=27�+�;S���4o.y�����T�*�H�*�����L�3��˟W8s.�ECT��<R0��S8�43��f͜1ma���3�XނG@��7�a�=mȠ!�������;t���F���07U��u����������<ퟨ�_���U�t����IG���=^Go���sA��g�-B��tt��n���sJ���]GO���sW������u�+u��:z��ut���:�YG��ѯ��'���~9]G北��ѓt�B����@G/������t�d�8�����W��W����):��u����������t�T�<���ǭ:�z}����Lh����g�����X�s��ݘ+�_�FL�B�@S�����54�i�?M�+�4v��4}
�����QLc�T��ALc�,��1�p�4݈il���4�����4����Y4��46��`���شi4���I�4���)8�~�؄�0����~��������Ә�I��'0݋�O��1�D��?c�7������>�~��#��R�i�L'S�iz,��Q�iz4����������O�C0�B��[0�;j?M_��k���������:ܜ;�^�Osfqbm�`z�-��)g�B��؆D9aG���~).�@�P��ߒQ9�>��"�N2-��7�$a�Y#0o�Q6��F���	�����&??���*&��Q_<P@�u�邮D_2,�줦^�ש��^�miαw��*B�)�z�AfX�<�JI���F콡�"�}� ����֐T�j�%ݎ *����z�ds��`x��ļ�y�2�<�&�p�9�@�BI�j���P�X��Y�����2$�z�.�1���p� �䄯��_��Y_�n=Ӕ�$j�H6��^��̉�E��bT6��L
3�AR�8���*+��f�>1����bg9i,'s�X�m���rRi�bn��l4�u������l0^Hu�J�y(Ejd7�̨1�7�A�����ʅ�C!�5Z��4l;�pV)�5�����N��U��`烚���B�hB y��,ʳ-�V�C�aIs�}�C�,��Q���Z�ء��������d�$�4�ۑ����8D)��/�����#a��X��;��p�B�.<W�[�-5��Y���Oy��w�E
��� �Ko���Tf����/V7u=�.|�Q������CX^��ע���s���}-n+����˯��K/�n�ƕ\A�Ղ#���L�oy�Pb�|�ex]�
��GM���ci�<Ƙ��TV�k"��6=��z�ٴ���m�ly�u�&۳쩲k9Rw� ��>ګ�q	$�7�y�'����<�]H-{�(�iM�>׫������S��4&L���)-MZu2�4���+�W`� �� 0|t!csɡ�>j3�@��jO����W>�%��9??��%��޻G����,ĥ�f�s���5{1�wI��.�������b�w��9�	�d�hй�X�Mk�sI���B^��W|p�;*=W�
���AJ�%޴�)\�6�03��x��@���z�z��q�}")�-����8��O��w)S��?�ZXSz�d;ϕ��u\E�u �l�1P�>ȋ��w����L�c�8S�Jʇ�\��u��}�⹒����!��d8����g�p�H�7���;&���
�i��5T�׵�����sK�⸒�|ׅ!赒�Q��MF��
@7�gwΙ
f��;&T����fU���Zʄ�Q�¥��G�O�+'�F�#i����J��y,�L.�@k��%�-s�T�O�YQSz�Y�N���i􏩼�Mjd�4���!�a �]�ԝ���XO"g�\��v�ϑE����]���`�F~�6��A�#'�(��-Ţ�O��x��@�%F,n������k��;��EϝF[�K�Q�{6���v^X�ؕ{����M�GM�� 2��et3�eC�ڥ�Dwj�H�F�q޵�@����,�w�ǩ�?6	0Θ�[��}�̾�K�w\~��G��,h��	�r2���A��Rda6
`�]��4l�ʄ6�U}�3@_r�r�&w.՗��ܻ3���P}4�4�Ү�uP�T)�PU:��*�=(�F�d��f��P�ʉ���`��'��A���:�������Q�?��{�MC���������wU���ԏ-ۆPڪ@�S�R���/�ʱ6f����U���ș�{O� G(�YTDF!��L�%���`��hp~��-��R0�H�~�]/��Db�]�'�� �<[��u5Zjt��k�������P[ yS��]�+`X0���s�TO�q�9Te�R>c����`�ؐ�Q�ԍ��7b�6DJ4K���yF���}��:3"_t��A����_�
��"��� ��Pܷè���^䩥�0z�\�������;9?q<F�"�A�4k#qX�:H��&?����Z��>ْǤ�����p�"?C΁�@��{C���&@?]5|N�dI��䁦r�~L�j�M�͒dm�I ���(}��3�7H=!���dA�CH�#��x�{@��ɑ<��EםNJ`IY�m�֭����>�9�2�8c��t�'��(��I&QvX�X卦�� W�d����S�)�T�aa�5���QN���ML�l>���G�+����������� ˴��ބ����*�U��ev@��1�pgt�ݗ�7����rg�&{V0��_��n��V�x���a�9H+�C���<{F4��`�j��Q��v�q�Z7���G#i�)Y��좯H�����k���p�c�d��&��ƍf�sX��'����f2�����:<6K"�n�����ys�Om�ʅlGJy�A��b�%�˼��,d � �<9�~Ǔ�N���S2�I���r.�Q�P�H��#��E�25{`�ڌv�:�DB;i���IX���hXl�V���nq�Z��(�p���1c$f���d�q��f'��U2�E���{[N��m���I���U�sx�u�X�x_��(�<I+�JK�?A�J ?a�d����8�aq82��Q��Y�&�+�$gq^���@&(c�+�z�FD��\#z��8��*]�QVB!A�-�^-7p%7	.�b d�W ���೾'q]5�Ѫ��C��H����D;��A�&���A�j�̲A/K��Ԛ�]ϴ�'tb_��_�$O��x2�Ϝ�K����`�g��,|��B�y,I��q\I�-�t(�|���"�c���&?���F����f�����ae�Q��"b�l�����<`7�� �Ie�,�`'N�|: ��҃�C�2�8��t�阉���z�6˂�Z���]������� h�7��XM~)t�Z@҂�<��4 �2�2���RS���q�lꇃ������B�{
DsE�b^�HΉ0��(�����Ϥc���w�p7��T�9F�S����G(]�cԉ ��xZ^kɳ�Uᰩ�˓-�QR-d�/B�U�N��&Tc�v�\6c0�C����TS9��\:��}�ҧx�Q�C��!^����K��0��A��N�mȂ�M%UB�t���>a��
G�t$Fc��U���	�R,2����Y ���1 �@h�X���:��4E������
IIF�g�Oұ�G��L��~��L��ЅhVg�?B'�*����(Xiw�q���B+�!�:'?�m��CP��t .at�K����ke�������\?�5Z��5�l�kCqѦS�Ū�+8%����Q;��^�,�p�Vp��A�g�Rl�A;ߵt	>0:3�7�驯(�Xp�6�n��h*����~�i��e��j�x����%4�>�z����v��yC�;P��p�稶'2�d,-�I�@�*?��w�(���npI���5�&į�`9���sO�0c�@���6�����,dm��!�ʣ�1�%���;"����M��`߬���U߰� "@�� ��N�-d�J�l� ~��}K~$� �Q+�dA:LQ���|��P�!�,�!"�\@0�"'���#T�V�`/KU�������P� ��c{�N�	���Ӳ�81��X��:e��%9��I�;g`�l��$�&z�@C�P5kAG01B�{)��= ��|��z�_�Pq�(�b�oQ^i��C~�RG�+-H/݈kpS�о�=lNˁ0q>4v�I��m�E������y(���I��H�z��ϱF0�G��)���C��ʫHɣ�)*~����]c��0��*����j���I7x�-�n�°`�u�
��r4�i:�>:泀���*�9��>G���=��&�_U1��w��RL�I/��p���DdK�oS�!-��k��+8��m�K��E6���sveX��L�k��\�jL�Ԅ�^�Z�(���������`l1�j�xo��OUq׬�?�~�Dom�LW)Q�R~u|��Rh�b�%���w2����` �i?�g���ނ�\4p�l��AV�V��G��,����Ƙv	��-��.����;`U-��J����/`AʫQ�!á��^�E������Y�ȫ|��oҟ��ET���Ay�!�M�D�o~F���N�Gʫ|t�#�"�Y��i
��Ds���s���*ן��C>h'��@�\<�.X��Z*;�{E2�yi&��<o�@�W5�l�l0���'���HF+��S����&<5"}k+��ӑb�x��ƶk�%u���S����d�IЕɏ���Iȫ�H�~��~?f(4w�W�P)JKkX@��3�B����= ��-G�)l�l%; TU�[�����Q��=��� ��9kH��h^X�3B����Tv"T����o�\��i|�������X|��	^`�<ｕʀ�vG���@g0���)�R5��D�}�$ F~%�lx֡p�H�Yg5 C�6�n���N|�ZC��ߙ˞��`�e{�L���T���hl�%���o�|�o����@�;�/K��o�
=�����oc�)�s?��G*�y��Q��Id�|��L?e���h�/K�h:?@P�Owf~ ʫ<w:�q�"C��q�)�n��S9����q^�~������j Y����d�l�F<8��FfD�pK>08����K�ݼO�[�u�O!�υ-��'�*�[:���u��Kǋ��xq��ŋ��:9r"+�-?�ay��f�[�a��9+��r�a���dOFg� +�O�� ��y����2<��#�����ă����:<Hcx�w����R���u�߷�:<Hcx��J����= ��=������9(��f]@�d���t�Q��*c��#���k��L�ݚ;�d�l��Y<3������H�V�}fH�� �f�����I�w{��.P�c �D\��I����mo���Տ�h��v�1Rh�EN`-r�r�+C��D' +��� �Wjfм��E�bN;�+f��"������+f�+b�k�w6vV�{�x�W�1��"�;;»��qo�N�V�;p�7��p|��Z�Jt��:1> �UF�M��1a�?;�y���^_�]4����n+�k�@֗9=�B(d*IN�z]g��	�����a���������Z@��wj�`��ٍ�����;A�+�#���lP}8�/���r�_ܿ��J�?�	!�U�cl������ַCqi�PO8��ַ�K�(���$3�W6�B��J#���e<Y�<8�l�*<�K�(�����s'����N}�#~�6A�t�>���*q���/4������m�lQ�a8؈�	�����*%�T��.w��5�V�T^y�~�O����L�@��u3���ǻ�(.���6X�nSҕ�k���kc�q$��`&���$G���������/Y�<� e�՟�}�@�鹦|׹!Ń�K��q��7��_7J��|qo�i��aBq"ܐ^��]�]�+Lk�զW�I&Z��??��+;��g�[�wO;�[�(��Y_!���'[�6'OU�xJY��萍����|�ۿ`Of\�N�$��8ޖ��9aP��sx�S�$z�L&ټ���$�(猄Ղ쮠I�imΐ|�}5\��5���z�i�{+ZN��ָ���2�C�?lǓ�O�[������/B!(�|#�:
�F�����vX��Z,;�f��[���D� mP�t��(���{�bz�Y�6>a5���0�A���\�����}�ƥJ'���2s[(��U���r�(��C&H�� 

�u�.��Ӡ���MQ�ѩ�#�q׳VYKA5l������L�.ܚ�^v�>�B��QO�6�_���
i�Ǥ[�67K�J�a8D��|��]�Ӣ�۪�����U��6�����ݻؼ��o�����T�2\��|�͍m[qC �s\u���UG�WWvT�o��p����q�=|to���a���p(���tk���l�_���VU� .�n�����c�5�/C��D�� ���ۉ3?o'b�_f�M@�fN�E�տ�����`{��[$���@#��2�`x3d�%7Co��P��y�a����Cvr�^}$-�L_:��3G����"I��&<� ;�Ek���d{�=�fI�����)���}���1K��D�����}���M�p�/�T��z�/E��W�B.u�/��s�fv��W���:8��1��X4[�90+�P�E�qє�I$	����؄��-<$��,�]��|�����c���@���v������i�G%k��W���w��E������lm�1�d՟I�k{^L2��]���hm����˱S��>KG�#�9�Jt���(�����I��Ɂ����#�ەۮ���6p �90������=䠫B�
�,�������� �b�F�����j�[J}#O@��k}�i�`�e�%������9:8:*�m�:�3��^l��Ue�����;�"�Uz��Ż,����B��5\~��N�Cʫ�������VT֒q�
���z~�5�O�����*����ʿF���Nǋ�-P.�{�x����P�BR�bFw����㼘�3���胀��]�j	�/�L�0��WE-��z>;�`ȹYsqcq�7��l;"��(܇�?�op���o���l�׋����M>�&�5.�����_|kL�i_'�9(��a_{�� �,�9��ڀZ����#��]�+�lif��a ���v���n����9��TͶ�S�6X�L٧EY���t�Qu���H��+D��_r_Ot�8�q���$�
j����ҟ>��l']�,64�}�je��W�����u8����4 �� ]��՘���`\0�tȋ,Yxl[F���Z"���b8�u6c�#�n������Jd��ߠ$�3�'�����Hfπ��B�
}��)�@�2�;�%����Ibی>���P��xvy�:
��Y�Ø��-�@��e=V�fћ2n�+"7a�p��dHJ�����i��dX��"�.i�d7�z�,����{�i�w�c�ux<���Q�_\��VRu;Z36/>d��N�!���Ô�
����p�w���C!�
��pnT��g,����4�Vz)������Gb�����lƅ���Z�8��n�����^<�-�'��b�e��Ņ��P�L�?Cu}� '�4��*�����ƈ7���*���B3-�C�@�6����w�y��������102��{��s9u�zz<^����@��u ���>�>g?�B��Z�4v�������h��њE{E@5��hV!jހUT��V����ڂe���`���m1��aB�Nt�I��C{V?{��plIx��uld����RY����H�Һ�/D��_�̇�7SN��ۖc�X�Sm���6�نڕ9ia�����D?�;ƺYz�n�)3��W�:d*������O8~T���Eo�U��OS(�S��^co�my���*�w�M�	���&��A9� g��i�S�nҌn�U�԰�n�h*��]1F�鍶V��r�������z��u�џ�]�19��&��<}	^��E�jd�B~�n�F2���0�b������s����)���5x��
Q�/ux���\y�x���f:�c�X��U���K�(�v��g�p���O��C�2����c��;�R��z<Hfx����æ@�D�b�$�H�t�צ4H�§�b�P������'��^z{l��׳S�8��T/�Bdo���h����OD\�1�r�aZ�����Ni���)�,��g�;�8��:4�=�m�])�a$�aHĆ�z0j*���[j`�F%������a}�R��D�z,|�,� Zw8��J��A���1DV��=E�>oT�]I끶T2��uZ��	�_n����vE_PT�0<B�T�Ӌ�y}�C_ڋ@8��⑈oL�T�`/m�(�T(��P���6�'|L��)_����\u9xP�wt4�������ѝ�1�<�,��i�n���Q\�H�6�����).�-�r��w�R����Mg��̴kic|��ڨJ�FZZ��d��/����������S���߅8��Y�����י�����������"�/_s�G�^:,G�LR�䮹���_�0��8k�o��m�r܇M�hD|�6�A����]؞Kkp��-/��ā���nɋ��r�v:b���R�V�rv�F��u~�4�F�C��]/�[��P�'G̞�F}ݤ<7��ɦϽUI�5���yϖ�w�V�`<jr����+���#����W\<��]ü1��PH/?�O��[b��lq̬U2��w>34#�vb��*��ƌ�ShR��	ݑ	���#�~��+|�"�(�OǑ&��P�C��ա�@r��lc%~�D�'B��13��Y����K���aD��e&fy����i�DK>�5B���@��~���nR����x�����d��>8�����P ��o�՟�p�mT��!�I\&��yƄ*�R\�=J�Ȩ�qw(��֓�x%uQ�T�ɎZ�U�w7����G�����/Չ�${-������m/�������Sowٖ��t7��D׆D�S	��24����B�J�Q�K�X)��U�����n3`Iu��_���XK�&;����4Օ� ��� k(��0�e�;EW��^�G%�+�.Ֆ�w~�_�őF��/C?�9֓d����$��q?�F�bW
��<_�����i��pٱ�InhD�%��wFy�#5�`�T�}������=�4[�+`����P��s�G���}��2�!M$�U�P�+փ�i}5�}�lT�,�fYH2+��bm�eV���2�󦱼cw�y�u����k�<6RC�z��u�B��cXo��u�����d�8����x$����^�<�K^+�%���u���������ҍ(ܐ�_�����5�,f�� W��=���zp�͒�������u�E!�6���Y���.M�6z��U=��>vϝ{�0���]369�;ȷ6r
��w���W��ᛓ�ȷ\3��%SH9]�>��+�?Xϑ��B g#�-ZR[f��~cs�$��y���<C��?
����[�����F[F���.���Q��,�>�ʹ:�PD�VX	w60 �N������$�u�KOP{KC�5A�sj��)OP����="�&e�\��Y�e��n�׈2#�2��t��w\п_)w�ϳ)��vW!���_Bߋ;`
�`�F�'X����Ϋ�7M���~Ӥ�f]����/�6*H�p��|9R�y�z�+§tNMP&���Iiv�>���Co�ꞗZ�����^��ƻ8ߏ��\��qK��}!�t'�-m��YQNX}'t2O�?�К��]�܄݋�.6Q$?�g� Dר'��3�}I��(B���p�����IN6*7���Z~9��މ��r�S_��Vj4dN��K��f�!�s���d�;�2�U-�h8���:B�9���(�_��&����z��/P��BX�DM�0S���]K��P`���e�_i�q�4�	N�ɬ�e���}�#$-d"]��?�{�6Q3�{���o������ل$�nPԴ�z��k��I�ػ��F�&�V0	ɆlM�1�!A#�n�n�`�Z˿����S�����J����j��Z�	��j�Zd�ߙ;w���&Z,��g�9�8s��{�ݤ�ӾϚ#�˺����sfS����qk���Ỳ�r���u��E�Q�'���V6>?;��'?���w��_�^j�G��]���p6]����Ye�7�uB�C*d��`�$��GsX}��oa&	u�N��~�t&\�y)V�{c�����)�k��s�(#��o��-c����m�U?8�"���_�:6������R��BV�_��*>=�p�7�o��
v���1��� Cԃ{|��P�[#�^�SղJV���ߡ�5����E�2ǚ����o�b�1����:�U�Zf��{�)3?�3�Z�k�e��8�rP5k��ڟc�3��d�+g�ܞC�cj���ѓ����Sxi�6o�:z6O�#��	^��_kj���НR,�䰗C"4���΄B6,�>R�V�ڷ_o�U���
�t�����+��Q�^Y]�G�\%���"
 �t�!�o�Tj���ZM�Z���ժ�7r�^y<t`�G�^��eUz;k��况�;s�m��J{m�U�kun��^�;l{�,W�i���z�0�5)w��������9��oY6Z������ZkK(b]xk��R�~E-ů�{��i�0���S���Q�әV����:m�Nߒ�^f��$��_Qy�.0�g��������i��%�����֏��~1�z4���F�c�+��ѹ
з�|��������Ƣ(�'���ip�p����W	w�L}�B0�Q����LS��!�|�� �=U$�N��������7e�� e�B)d�T��֫��A
I;�ީk�� m�Zvc������1q�G���hP��Ρŀ^��Ppe���qϡ��=�?�}1���Eam�J�MGF���N�z(7����X���Ϧ���&ó̄FJHeC6�����B��5��^�Z�d#�`:?A:���S�r�wXy�gG�G���5���X�E�vp/�w(ou`�����XiXs�y��
���/oy]�xӲW���R}�0{J����SR���jv̂P��n�C? B��S��u�f�����W���#�'��=�Ph9���n����4�S�������+��m��5���V�Tu����?�8�\o�,�2ìn������?��6��~��1�.�<��C���{�,�׏�0ї�Ѓ��3�ב!��i�lx�����<J�糽l�\єYdѬ�#=ֽWzJ'g��!Y޻L��T���ǃJ��n��"�Ŏ�Q����w�SЗIO���p��v�w>ԖkO�-�= k�ǲ��2�H��2�f�_�q�Id����2EU��2��/z�����*3�e�JG��t���u��6���Q+̊4
�6�>K��H�5�ʼ1�{�����U�`.�$n����l%����U���E�>�"f���1X�o�����E��Y�䷼��:d=��+YH8�=�&�߲����a��X}(��*[�9�I�4E�WL�n`��CG�:�&3&�g���(�y�y�sC���ۆ��O����CKM5;�������G�t��c�6�� M��0���y��4z�g��-����g���3'��6�iFdp�9��M�L�����0��)	�s�����u�@��B���>�#Q/	�~RH�(�<Y���ͬ͌7��,}
@V�:���+����w��tgy�����njg/��������*W�y�ެ��ǉhK�|����d��L/�^"a̈�@���gյ'P��)T�rW(9<����S�S��$H�ʗ1eh�^�J�;i}��xu��0>ǲ����E�1�4����ӓ�>Ѭ?���m�ک/����1&�YlN��*�a*����\��X��V:��}ӱf�ÿ ��2�v�A�w&Xvy���6�ߪ[~a�ԯ�����wF�ǭ�*罿��	=��B���-�m���K
���͜�i���ΟԞo6��s\��������M�ͤ>��/�*���|��[��gj/�j����ރ���	�I�s̝s��`Q�K���";�LO��,{H�����I�mv�Qӓ����ÎΙ�Y�mzHy�u^w�%��;����h����N�Z><x@����$��5{���z��8��Vq�_0��\��=���MO�a�6T�A �V�C7��"[Y�������(��6v�_��\��~�|���{(�++m�_�[��>��r��LO�V�mH8xI�]�e������ͣ��8,{�t�v��O)�n���7|b�mG��N�f�e�ݿM�m6MdE�%-a�=t^�-���"�:��H�ر�N���E>�� ��sY=�Z>���A�L�;ؑc���g���VFfd���ݩ�š:�w��K�u�e�C��S5�<�������0䩪{>�^y�����b궃�*���m��/_Z�+Q�<-ry�|�ʓ'�'��#Ry���^�*Q�w����P)�BA"�R��P���\(Q.T��B�ȅJ�P�|�Y>���%�"�5��I���O�ZA�����b�H:g������L}�\|N���`|��s������������w���{C���ۺg�j��cZC�cr��S�ޠ��8xݨ�N��(���b�c�&5���y]�}�ܤ"��K;�;��@zg��5�������6/&ۖ�ݴ���m̾�D��E�m��/4���������^���m���gwưh~i������>�ӣ��B[H�ݓ~~��G*�	�6�*����,?��v�\4uGQ߱�+ּ���!f�=��ʘ(8�[��y ҕ���_������=�� @P�Q��ˑ���C;��a�R�!������ ��w��N��z&�`,!�?dK�����O_�X�}�{�푿[��_[[�\�.N��i�bu����U��<��[Yuy����g�D;�^�8��=颌.��*<XXȫ*y|f�d�5�b�kM��Q�YOPN�'A�-m��JK]7!���gţ$�w�X�nk�u��B��V��􊥅s�Kk��Bz�1��F�J��U,�-K���u�d�T9`|x�j}�Od���"8����U��r6T�u.�W����
��ƌ*ws�����:�ν�!�j^��j��g���ړϿ7(��\!N�Ԛ*zPH��0�"-t5P�j*���D�D�EB���!67\]Y窆T蓄Nʕ}ieu���j���	�=+<���`�[�)[�褌�:��]$TUK�o�_��\�@�Eճ�k�A(�8�yn���_T�����yGy&WO]����)^�&g�;%s��u���#N��:U�s7,[�l����uڴl�E�O��*��&&��uͤ�H3Gt7{Ew�H�jZ!dk�Y�E��!�e�uJ���X�Bx�ME%�����S�,�W�r�XTB�'�wِ�"�tE$�>���#ܻpo��	�\?�+p/��A�Ak�p[����9�F�����#p�= �c��������:�n��.\\}P�w#\\=}an)��������p'}��@�S�^�81Z�΄��.\�v�	���7�"�����l�𘖼N� � ���	M�L��u�ŀ� �C�I���\�	�p�!̭c���X8p�b�?�|��� S�'8F� �0U��� �`����Et��ŀ; � ���p��8�r� h'���p�p�F�p�������~y�8�M���@;�7!�z���x�a�uO!_�37#_� �$�E8���m� �c�^�?��p���!?ী3� ?��L\�+�`�s���oE� n�D�^���S��}�g<����m���j�o��v�<�� w nx>x�x'�و| �7�\ � ~�-��>��.����.��=Hׄ��t�a�ː`�+�`��ث�p��S�q{!G�C�� ��rL |���� ?�!@�4Aۛ������z������p&`�o�8���\��;�p1�>�q��I���B|�f�u�3S�#�6@/��C<�� 7��/�"<ำQ��Q�{>@= n� �@� ��~&B~��>`�(��Q��H�O��!����|�z<���"�mA�D4��z>�r ���wz�p`��w.����p��PN���[�o��n�Jе&��7&���$��B:l�fL��ۍ�EF���Ei��{�=zv<+�sEG�Z}�Q�2�L��sI(T�s��W��k�u��R������^����i?��i��Q������f�ْp�g|z�>}��I��><�1��y�0C�D���>��D)�����O��Жk1��_����$��N7�x	���2&�2&�2���)]�݆�Q���2m�<���7�#&�J��C˱x� 5I�Χt��E<�����))��b��J������o;����r���FQE����A�a�v�����`��57H�Uć)~�	r~����6�8�ث�:��r�A��$�iP�kmT�Q�68�)6cfGLW��JV�J�@�dI�,��3�.}�n�g�$|O�|ѥ��n='���h�ط�+}�.}G�cE������NWv�ꊢH���ur�A�?Yb�p
}Q%F�C��G׬�X
��2�i,��:���Ls�.�D|Q�7@^�S$�B�9r�a��F��g�.=�z���AOH��VM|e��D��Q��C��~*��
�x���M����ب��Pb���Esy��t�$t�>SUR����o����ɻ#����Ft�5�O�=!����tt����3e��F�oҖ�����v
���g�o���ߡ���K2%ၠ_�cV�$ �P>w�~m�zf�zf��t;�#�i�Z�������p�$>�V�H��a��B|
��gX"Wu�����)�.��FQ1�~���q��vh�h]a��lB�����[�����2�Kr#ә�'ۓ7�t�*o���x]ȗ��>�1f����a��;5|4�����и�0tV~���p�&��{g���G�9���_�͔�����e�ޘ�V�틑��~Ћ5�H���������D_T��(�i�!]&	��c�S�����A�u�!�+�)	O#�q�w$H�}D�����Š��$ak�@�:��$<��������0�/E�+B�?�� �h���H�����?�*
%�y���^����H�Ih��i��#���O�+	��Va>*���{�j�t�A_�����u�tZܬ߼]��X�6�����t߬4߫�'	?��~��u�$��}\H��V?�X���w�Œ�;�^;Դ|� *���+$a��:��s1.�?�3�K��0N^��Z`,�f�'�I��.S�w���@��/�?�����S����Y�h|Rl�P�A��U���0tz5�0��xx5ɠ����Ru�+1&��Eq��X6<��V�獪��a��/��Oz��̫%�j���ݤ�ڌ)4V��0t1�<D��Fb<�q����k��Vᓀo�Vb5��W]��U��y�a�4v���·x�Σڹ�$�����m���2����~���W{1��j�O��y6��x:�5_��O)=�g�����2шtA/�{6٩9���]�^u����^zE�$��"�{AϬ��7u�x��V
�3��=�
�:�&�_�$\��	����H��~~��:�*�o�`'���G�����S�r��(���b��d���%	G�S=V�~����}Xl�]e��tw���C��zng��MR�ކ�l��dM=� ��q����_�u�7/�<��Yd!�l�O���Ŝ��_h�~7�GU�R��l���x|�ֺ���(�b����	��S[�	^E��j��O��֗�$����5����<��x����	�S^��/�/DZw��e���@@������
|�l�Ж��F}�Q
ull���?.1ºy����o�ҥ�q�;V���-����z�����?F�sq�Q�_�c^���J����G���;�۠�l��rӟҿ+k*c�0|-�+�Y�}q��nF��ܯ��0�#	_����%a~��%�����&�px�Øq2<�Ë9�r8�ë9��p9����G8|�Ý����0f<O�Ë9�r8�ë9��p9����GƏN�	y��bA���ހ{	n7�v�����~����[w3��焻�����ӡ�������n6\�T��pX��
�����~��Kp��~
w7��V��p����+_�����/��p�p.�epK��-�+�+����7�kp����}q� �{�cgϞ.�̝�p���nI��23s3-�ibJ���B�����:������sVz�S�������K��z�M�ʥB����lj��^g�mVQ��r��6�
鵕�Z!�zE�gE��MB�������&����)�����Wc���q�_��������Y�����zgymuS�'�Wy�M$(�oW5��+�]UH��e��q��,� X����������X�G(�S�����P�~V�;��U
�/�Qޠ�_���r~�>��?���zM���8^��c5�dq�5�}#��+8��e�����y_�׌�c����o�~�>6&��W�����W�J����6����\z�x�66r����j��������N4�i��x�b4�{ǎ���5�9�ƱAi�8l����y#0V�j�eM�+5���r�r��	�߮i�8�YàmO]��$e�Æ�ӿO��>����?4!<�V���x�hO	����\���=����4���?����k��y�EiƩ"���	a�X��j������9�
|^ï�z~����h���q��;e�nd�y�ǥ�Kw���w��G�G��M���яl��yW�Y��{'��]���gC��M#�Ӧ1�?�X����%+;3;ے���+dZ2ss�1��@��[�$�B��=��D����w
���u!���������Y�F�B�$�	q�c8�ٯhU8I���Q�cU��x04>��ƇI���S�����7���{�`_��;J��5<���0>=�K�|)<�;ޣ:4�ƫ��f�DM���)��иU
�$j�J��f�4qy��0X���5|W�/�$�E)_F�k�5'��:���Di�y�4kN�ǝ���N�2�T*L�^izĦHV�S�xM|\���zM��>v�7��q,_�3��<4�9K��/^��5���X���w25�w9����B�K5>��t�A�i�_S�3�xe�4�:�j�����sU2#ȭJ��ze��B����W9��<�-Ϯ��x���|Y=f�d����T��4�*46aV]:M��u�l��PYk]�t6����B�|l�㮺��\Noye5&���p�F�@���M����z����r���x����:�]�l����0��h������i��F�O�E�3��c�_w��q�������T�x���h��Ux�g@�W�k���1�Q>�^�«�}�
?F=/Q��c�D^�FE^}~*E�����T�<^݆�ՔI�=�2����[`��c���c���&��M������?��D=����O"�e���O�����%?�tp=��%?�r���_$?�pp�o#?ew����%?�l���F~�`	�?F~Ѡ����Dþ �|�I$��j_��a������������=<�޾��v5b�<������^�k�t��׫�3Q���V\�t�&�M8hOo��=���̩c��=�3:>�n����_�[���	"8�iVEHk�Vd��n[��UI���}��Ա�н�~_`�w��Ӻu��.%��.W� ��7�++3�<�ٲ����J��S�(������^D���x��]z�-�B'\������̜w�R������t�ܿ�������\���������=½�_YZܽɼ
٣����LOٗ,5�����v�m�,#�v���K���tu��s�=Ir������g�^׮v���Aq�s#]j��q5��,����^�x�]�o3�Q���n��g�ݿ�<�Gq�*�����E�2&g��2����E�.�U�~?�������|�����KQl�Vi�Ipto1W���3ؓ'vs2���.6K��Xf�� Jq SO�[=��Yz�f``2���~��q�]��$<z̯���ͭ�!�u�^9�qz��s��(=������ǽ���&�=�S;5�����5�URс;���?lM�������P�7�;��'���Y�����1����.�nh�������hzj�mdڌt����ߣ�w4A�%��~��^���j�}A�a��+I�,�U	��@��Eb�x��6��`)�Ʉ�d�ޫ�Nw�鎏���}]�=��������Z&Z�؞&��,���H��ϷN���!������jz���|�o��Yv5�޳ds)ɵ���䓟�$'I���*��	a������b�:��f8��Ѝ��R�;{�gW����Ү蟆~>�<y�����xwF�	�7'������}��
`l�]���UH�r��������kA�,4���(Տ���-xL�����I�{.}g1z6|+'�6�|P�}z�N��T�t��6m��zui�l{���=G)��I���G>�k���ݦ�>���_����7��B�N)V���9��^������Ɠ���^zOH����z}��tP�s)���v\���ԏ�>늣�>�s��q�U}_�B���z4at�U��5x_U�����J#¥�5�����H�6���b���e��&��Ά�����8��Q��Gw����'�љ��L��Xf�^J���َ� �����tw|����lj�Q߿������C��|4.�Џx^c���A9���C�J�U�T���sɰ{(���&����9l���Ro��P�?G��;�gL�����:B(��|H��χ�|1����9��1��@vG>Rk?2�?�Y8�s g���Ӎ|�5:�HE�q�e�s R���@��/�Hp^1!�����z��������sx��9���Wr���s�%Q���	�gW��ە��ߗ/;�}y��*�ɫt'�/���ZC���b/��p��v_^��R����+��":���ݗW������g���J��1����_��/p�~�$��ߨ�sr��J��xj���9�~��	�����u�p��|����u��ۗ�ھ|�O%�v_^���Sۗ��ھ�����Y#�?�<�������ۗ��:�tv��{�}�
��ي�НҾ���՝ھ����ٗ����c�����[rs��oɱ*� 2��,��e�Z��Y�9�Fv��_�����j+�+������tQ�RWC��6��V�(�h�4eԹ�*�2<�OW��������'>����ʋѲ�^K�s|����-&�3G��"^G�a-I_��;#���xW�x�h�$�9o��K�����G1nv$3�h99�Y�m��lE(�3>��1����AEU�W4+��6���q��5�W�e�kf��93\�^:�fyo2��z�-S*��PP�lml�k��V4#���I��f�;}�u:H(Iz���X�l�Y�9[4��i������8�-���u���|�''s�5���X�rr�ra���s���m�K+�1`��ֺ<�������-�4���7R�*H6����DK�g���㫛��a��V�!5�y#G�@�r�r9X���!d��q ����_hVe��04�3'^z���Y�]l��cv$��`J�����XJ�!�dQe9�R�M�UN��JomF������[N/ 2[���t��z~�e^��%'�%/��b]"^W��x3�^���ʰ�,�D8	�eNo���Ïrz�t�� ��9T��������r�"��H��i���<��&T��QM����	�
�P턷�i�:���C���&�)������}���3�D�TB��HE5�PKz�pqrz�ҁ.N�"����h}��֒�d�YYU+6�[�q�
��
�7$H�6��:]�bՄ�<ȁ ��N�9aX ��TM�F�E�Q�	�M�q��T��孥7 �i�0Ȓ���:;K\��M�eM������QV9w�R���5g>�4����Q�T���K�R��땕����Yhedˇ�����D��j���kİ�a->��]��p��nD��,#<s���T�F�N��BQn�=NƇZ�������D�:=!uA
!#��W�p+آ��+�3�3��:�B5�p�&q᳏�Q0�sMG�T=|4��YK<��El������ʞ3��^z���MO���)�����l���0	��կhqU{kr�'�rU6���L�
��Y_�����&��,u��*Q�H�E��������Cu\܌��זF�S�,�6�5�j��ȗ0��#H��SY��2�i�Мf�R1�jy���+�� �5h� ���g,gh�rxƲ�ڲy�a5�$i�i�j��F���c��L\��1K'33Sl��|�5"�,X4*թBn��ô��aC�C\�n�1��0���ɰM��U��0�sʰC~�M\��l�U��'�i6g��� N�j��W�椉S���l�e����i7��ެM���%fϔ��J�Ŵ�!f��'�5Q�3lz3��¨�wE劥Γ"�=D�.��TZLr�X �k�*On���*�y[-bZK���\�7e��l)H�lM6[f@�� yCs}��2u��ri��d�V��Sf��0N�K�k'{��7$�"�T�TsN�9;՜5����.g�����є�,�U�,V�j��H�0e�e=1��p�����ⴆٖ�睾��� �
?D���Tsn��:\Ǒ�]����A��t\�lpG_�C��+V�!}f�����W�(2=bUb.u��%"�2js������-@Y�� i�����d����t�7�'S��][:m�5��d��c�@݉�ָ-\��\�q��j��Q�@a�q�i��3f\";æ�'������@Z�:�I`�SV�<�$;���tR3@���pT9��ꬍ��j�y&�����is��P��?�;ݵ�՛������}���U��ޚ<�f|�S���{g��|	��3EmN�\/�-,�B%��s6����5���uG8���]mOG�l~���X�	�	mjr�5U�6���d�!ܳ�M@��o�̾��l�M���		�ξ������>Wߗ���a�D~qY��X���ZOf�lL���>�Rۯ���n��v��~�n�d���ܽ��g������
��i�՛K�����ԋ��Wr8����͢<"�M����᛽������+��׵Ɉ�y�="r�ō��$��;Ε�!S�DN������s��H?ԥ�)7����wYFv̈́j'��m�(�P���8�)	����ȟw��u�$�D$���a��pt9��z�TX�|Q�Kr���Ik�: eQ�j�l/���M�
�g��>���{ŵq���h,�����5�������	��x�[�^��@�QP��A1(�+SGW�.F�^ATX��;�}}�%�>�����7�y�yf��<(i�ZiSMV���3�=K��_`�_lB���ݓ�y�ˇ"m[5�쳔�Ũ���T���_�]���d��R+��(3����i�kQ�ė}���xt3��r����E�eU�����>��D��_P����c��b�$����{)�DB=i�?��>��/O��u�O�����&}gR�%�;�\���eқ�X�;r�i��+���I4�;����?{K;c�C�bU�u��?���+�a���}�o�Ϫb�-�"��	�sR����B{AB�ь|��r�lICxu:%]^�&��oHa���~�LU=�ZSh���S-o������t�+m���k.�%nӄg�\~��q
�[���QG�t�?��i�䴿���!N�ņO_gĬ���O���U2����ATە#�,ᖖ6��i��{Xz1,�Ka����to�ڎ\W�V@�hՀ�bk�O�O���@x����{��O&#�Tie]Vq&�zT��\L��4I��Ū�����iD�-�=�ZtZzu�)��Oú]W#�fd%�
%%+��n��V�D��"=&��k�;���V$#��� �Ǎ ߙ�?�gs�vV������S���Q���S�=��N�db>!L"e�DU�Jɗz�6�����]	T�,b��+'�C�TcivQ�m=_���"V�3҄ �$�yZ��;�8]���|kl�gck]b��>2�?o�J�=w�&�,gfY��d��<��f��-W��̚�����|S���r7#�f�D:���D�%yq;80Z�O�5n��vZ�B8�^g��(%nÌ�Jv>#l�3�p�f�!���A�¶��v
n�K[�mo~o�����3-�Z�K2���z䐬 %���(̾B!�!�[[;��sm�*�����1�k+������l���a�Z�x1� s� �Ü���˞	�'��N���L��~��o(k|���RN���!��Û_���k���r5]�E�<�H�`��r����v]�W�f��q�[��{�n{Qc��l�g�[갻�R��ev>]%{��R�KKB΅�v�k��'8�7V\�sG���4[��!�)_]i���*��S�.��w�U�L�����i������<2����'��A���i��� ���� ��?���� ������/� ���� ��?���� ���� �v��� ��?���� ���� ��?���� ���� ��?���� ���� ��?����<
���� ���� ��?���_�� �@ �@ �@ ���/N�� h 