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
�������t�{�9�s���;wf�sN�93�̛�����7��>|8�¿��z����9됡����}���`���si�����E�#��Ҹ���I�T�R����{*�qO��N�qwq����,��R��+"<Y�H��M��@:AW.�ˊ������Q-?~#�Xzd\V�5E-�]
XE�}�����7A�K�����t��|��'��k�V1|��I3�~�,=㉬��jA����|]:�^4�n�]��n�=���s�n]0rĭ#�
~��~��^���_�]����w�~7��K�����tm����?���;��F���k�}��Wm�I�~�¯���{^OOCt���k0��-����ߨ^��ПU˗��O��Ծ~\��Q���о�;��P]��(���z�0����gY]�ƅ1�� �^�V����^�������1t����}��$����?;��>�~�zm�|E��z���]1�~��_j��j�k�aCL��s\�v����PC_��w1���N�ԁ]�;(�M[<큙�I3�g�~��hf7mڣs�͝�3�4mCW�T#��Ys��\����s�|(X$͗�͞����>Y3�<:S�/�7_�����8���
r�3�ϟ7�+tBF��y�\~��yE3�|Ub�T0g&�̐�/ʚW(̝��=27�+�;S���4o.y�����T�*�H�*�����L�3��˟W8s.�ECT��<R0��S8�43��f͜1ma���3�XނG@��7�a�=mȠ!�������;
�����QLc�T��ALc�,��1�p�4݈il���4�����4����Y4��46��`���شi4���I�4���)8�~�؄�0����~��������Ә�I���'0݋�O��1�D���?c�7������>�~��#��R�i�L'S�iz,��Q�iz4����������O�C0�B���[0�;j?M_��k���������:ܜ;�^�Osfqbm�`z�-��)g�B��؆D9aG���~).�@�P��ߒQ9�>��"�N2-��7�$a�Y#0o�Q6��F���	�����&??���*&��Q_<
3�AR�8���*+��f�>1����bg9i,'s�X�m���rRi�bn��l4�u������l0^Hu�J�y(Ejd7�̨1�7�A�����ʅ�C!�5Z��4l;�pV)
��� �Ko���Tf����/V7u=�.|�Q������CX^��ע���s���}-n+����˯��K/�n�ƕ\A�Ղ#���L�oy�Pb�|�ex]�
��GM���ci�<Ƙ��TV�k"��6=��z�ٴ���m�ly�u��&۳쩲k9Rw� ��>ګ�q	$�7�y
���AJ�%޴�)\�6�
�i��5T�׵�����sK�⸒�|ׅ!赒�Q��MF��
@7�gwΙ
f�
`�]��4l�ʄ6�U}�3@_r�r�&w.՗��ܻ3���P}4�4�Ү�uP�T)�PU:��*�=(�F�d��f��P�ʉ���`��'��A���:�������Q�?��{�MC����
��"��� ��Pܷè���^䩥�0z�\�������;9?q<F�"�A�4k#qX�:H��&?����Z��>ْǤ�����p�"?C΁�@��{C���&@?]5|N�dI��䁦r�~L�j�M�͒dm�I ���(}��3�7H=!���dA�CH�#��x�{@��ɑ<��EםNJ`IY�m�֭����>�9�2�8c��t�'��(��I&QvX�X卦�� W�d����S�)�T�aa�5���QN���ML�
DsE�b^�HΉ0��(���
G�t$Fc��U���	�R,2����Y ���1 �@h�X���:��4E������
II
��r4�i:�>:泀���*�9��>G���=��&�_U1��w��RL�I/��p���DdK�oS�!-��k��+8��m�K��E6���sveX��L�k��\�jL�Ԅ�^�Z�(���������`l1�j�xo��OUq׬�?�~�Dom�LW)Q�R~u|��Rh�b�%���w2����` �i?�g���ނ�\4p�l��AV�V��G��,����Ƙv	��-��.����;`U-��J����/`AʫQ�!á��^�E������Y�ȫ|��oҟ��ET���Ay�!�M�D�o~F���N�Gʫ|t�#�"�Y��i
��Ds���s���*ן��C>h'��@�\<�.X����Z*;�{E2�yi&��<o�@�W5�l�l0���'���HF+��S����&<5"}k+��ӑb�x��ƶk�%u���S����d�IЕɏ���Iȫ�H�~��~?f(4w�W�P)JKkX@��3�B����= ��-
=�����oc�)�s?��G*�y��Q��Id�|��L?e���h�/K�h:?@P�Owf~ ʫ<w:�q�"C��q�)�n��S9����q^�~�
�F�����vX��Z,;

�u�.��Ӡ���MQ�ѩ�#�q׳VYKA5l������L�.ܚ�^v�>�B��QO�6�_���
i�Ǥ[�67K�J�a8D��|��]�Ӣ�۪��
�,�������� �b�F�����j�[J}#O@��k}�i�`�e�%������9:8:*�m�:�3��^l��Ue�����;�"�Uz��Ż,����B��5\~��N�Cʫ�������VT֒q�
���z~�5�O�����*����ʿF���Nǋ�-P.�{�x����P�BR�bFw����㼘�3���胀��]�j	�/�
j����ҟ>��l']�,64�}�je��W�����u8����4 �� ]��՘���`\0�tȋ,Yxl[F���Z"���b
}��)�@�2�;�%����Ibی>���P��xvy�:
��Y�Ø��-�@��e=V�fћ2n�+"7a�p��dHJ�����i��dX��"�.i�d7�z�,����{�i�w�c�ux<���Q�_\��VR
����p�w���C!�
��pnT��g,����4�Vz)������Gb�����lƅ���Z�8��n�����^<�-�'��b�e��Ņ��P�L�?Cu}� '�4���*�����ƈ7���*���B3-�C�@�6����w�y��������102��{��s9u�
Q�/ux���\y�x���f:�c�X��U���K�(�v��g�p���O��C�2����c��;�R��z<Hfx����æ@�D�b�$�H�t�צ4H�§�b�P������'��^z{l��׳S�8��T/�Bdo���h����OD\�1�r�aZ�����Ni���)�,��g�;�8��:4�=�m�])�a$�aHĆ�z0j*�
��<_�����i��pٱ�InhD�%��wFy�#5�`�T�}������=�4[�+`����P��s�G���}��2�!M$�U�P�+փ�i}5�}�lT�,�fYH2+��bm�eV���2�󦱼cw�y�u����k�<6RC�z��u�B��cXo��u�����d�8����x$����^�<�K^+�%���u���������ҍ(
��w���W��ᛓ�ȷ\3��%SH9]�>��+�?Xϑ��B g#�-ZR[f��~cs�$��y���<C��?
��
�`�F�'X����Ϋ�7M���
v���1��� Cԃ{|��P�[#�^�SղJV���ߡ�5����E�2ǚ����o�b�1����:�U�Zf��{�)3?�3�Z�k�e��8�rP5k��ڟc�3��d�+g�ܞC�cj���ѓ����Sx
�t�����+��Q�^Y]�G�\%���"
 �t�!�o�Tj���ZM�Z���ժ�7r�^y<t`�G�^��eUz;k��况�;s�m���J{m�U�kun��^�;l{�,W�i���z�0�5)w��������9��oY6Z������ZkK(b]xk��R�~E-ů�{��i�0���S���Q�әV����:m�Nߒ�^f��$��_Qy�.0�g��������i��%�����֏��~1�z4���F�c�+��ѹ
з�|��������Ƣ(�'
I;�ީk�
���/oy]�xӲW���R}�0{J����SR���jv̂P��n�C? B��S��u�f�����W���#�'��=�Ph9���n����4�S�������+��m��5���V�Tu����?�8�\o�,�2ìn������?��6��~��1�.�<��C���{�,�׏�0ї�Ѓ��3�ב!��i�lx�
�6�>K��H�5�ʼ1�{�����U�`.�$n����l%����U���E�>�"f���1X�o�����E��Y�䷼��:d=��+YH8�=�&�߲����a��X}(��*[�9�I�4E�WL�n`��CG�:�&3&�g���(�y�y�sC���ۆ��O����CKM5;�������G�t��c�6�� M��0���y��4z�g��-����g���3'��6�i
@V�:���+����w��tgy�����njg/��������*W�y�ެ��ǉhK�|����d��L/�^"a̈�@���gյ'P��)T�rW(9<����S�S��$H�ʗ1eh�^�J�;i}��xu��0>ǲ����E�1�4����ӓ�>Ѭ?���m�ک/����1&�YlN��*�a*����\��X��V:��}ӱf�ÿ ��2�v�A�w&Xvy���6�ߪ[~a�ԯ�����wF�ǭ�*罿��	=��B���-�m���K
���͜�i���ΟԞo6��s\�����
��ƌ*ws�����:�ν�!�j^��j��g���ړϿ7(��\!N�Ԛ*zPH��0�"-t5P�j*���D�D�EB���!67\]Y窆T蓄Nʕ}ieu���j���	�=+<���`�[�)[�褌�:��]$TUK�o�_��\�@�Eճ�k�A(�8�yn���_T�����yGy&WO]����)^�&g�;%s��u���#N��:U�s7,[�l���
}Q%F�C��G׬�X
��2�i,��:���Ls�.�D|Q�7@^�S$�B�9r�a��F��g�.=�z���AOH��VM|e��D��Q��C��~*��
�x���M����ب��Pb���Esy��t�$t�>SUR����o����ɻ#����Ft�5�O�=!����tt����3e��F�oҖ�����v
���g�o���ߡ���K2%ၠ_�cV�$ �P>w�~m�zf�zf��t;�#�i�Z�������p�$>�V�H��a��B|
��gX"Wu�����)�.��FQ1�~���q��vh�h]a��lB�
%�y���^����H�Ih��i��#���O�+	��Va>*���{�j�t�A_�����u�tZܬ߼]��X�6�����t߬4߫�'	?��~��u�$��}\H��V?�X���w�Œ�;�^;Դ|� *���+$a��:��s1.�?�3�K��0N^��Z`,�f�'�I��.S�w����@��/�?�����S����Y�h|Rl�P�A��U���0tz5�0��xx5ɠ����Ru�+1&��Eq��X6<��V�獪��a��/��Oz��̫%�j���ݤ�ڌ)4V��0t1�<D��Fb<�q����k��Vᓀo�Vb5��W]��U��y�a�4v���·x�Σڹ�$�����m���2����~���W{1
�3��=�
�:�&�_�$\��	����H��~~��:�*�o�`'���G�
|�l�Ж��F}�Q
ull���?.1ºy��
�����~��Kp��~
w7��V��p����+_�����/��p�p.�epK��-�+�+��
鵕�Z!�zE�gE��MB�������&����)
�/�Qޠ�_���r~�>��?���zM���8^��c5�dq�5�}#��+8��e�����y_�׌�c����o�~�>6&��W�����W�J��
|^ï�z~����h���q��;e�nd�y�ǥ�Kw���w��G�G��M���яl��yW�Y��{'��]���gC��M#�Ӧ1�?
���u!���������Y�F�B�$�	q�c8�ٯhU8I���Q�cU��x04>��ƇI���S�����
�$j�J��f�4qy��0X���5|W�/�$�E)_F�k�5'��:���Di�y�4kN�ǝ���N�2�T*L�^izĦHV�S�xM|\���zM��>v�7��q,_�3��<4�9K��/^��5���X���w25�w9����B�K5>��t
?F=/Q��c�D^�FE^}~*E�����T�<^݆�ՔI�=�2����[`��c���c���&��M������?��D=����O"�e���O�����%?�tp=��%?�r���_$?�pp�o#?ew����%?�l���F~�`	�?F~Ѡ����Dþ �|�I$��j_��a������������=<�޾��v5b�<������^�k�t��׫�3Q���V\�t�&�M8hOo��=���̩c��=�3:
٣����LOٗ,5�����v�m�,#�v���K���tu��s�=Ir������g�^׮v���Aq�s#]j��q5��,����^�x�]�o3�Q���n��g�ݿ�<�Gq�*�����E�2&g��2����E�.�U�~?�������|�����KQl�Vi�Ipto1W���3ؓ'vs2���.6K��Xf�� Jq SO�
`l�]���UH�r��������kA�,4���(Տ���-xL�����I�{.}g1z6|+'�6�|P�}z�N��T�t��6m��zui�l{�
��ي�НҾ���՝ھ����ٗ����c�����[rs��oɱ*� 2��,��e�Z��Y�9�Fv��_�����j+�+������tQ�RWC��6��V�(�h�4eԹ�*�2<�OW��������'>����ʋѲ�^K�s|����-&�3G��"^G�a-I_��;#���xW�x�h�$�9o��K�����G1nv$3
�P턷�i�:���C���&�)������}���3�D�TB��HE5�PKz�pqrz�ҁ.N�"����h}��֒�d�YYU+6�[�q�
��
�7$H�6��:]�bՄ�<ȁ ��N�9aX ��TM�F�E�Q�	�M�q��T��孥7 �i�0Ȓ���:;K\��M�eM������QV9w�R���5g>�4����Q�T���K�R��땕����Yhedˇ�����D��j���kİ�a->��]��p��nD��,#<s���T�F�N��BQn�=NƇZ�������D�:=!uA
!#��W�p+آ��+�3�3��:�B5�p�&q᳏�Q0�sMG�T=|4��YK<��El������ʞ3��^z���MO���)�����l
��Y_�����&��,u��*Q�H�E��������Cu\܌��זF�S�,�6�5�j��ȗ0��#H��SY��2�i�Мf�R1�jy���+�� �5h� ���g,gh�rxƲ�ڲy�a5�$i�i�j��F���c��L\��1K'33Sl��|�5"�,X4*թBn��ô��aC�C\�n�1��0���ɰM��U��0�sʰC~�M\��l�U��'�i6g��� N�j��W�椉S���l�e����i7��ެM���%fϔ��J�Ŵ�!f��'�5Q�3lz3��¨�wE劥Γ"�=D�.��TZLr�X �k�*On���*�y[-bZK���\�7e��l)H�lM6[f@�� yCs}��2u��ri��d�V��Sf��0N�K�k'{��7$�"�T�TsN�9;՜5����.g�����є�,�U�,V�j��H�0e�e=1��p�����ⴆٖ�睾��� �
?D���Tsn��:\Ǒ�]�����A��t\�lpG_�C��+V�!}f�����W�(2=bUb.u��%"�2js������-@Y�� i�����d����t�7�'S��
��i�՛K�����ԋ��Wr8����͢<"�M����᛽������+��׵Ɉ�y�="r�ō��$��;Ε�!S�DN������s��H?ԥ�)7����wYFv̈́j'��m�(�P���
�g��>���{ŵq���h,�����5�������	��x�[�^��@�QP��A1(�+SGW�.F�^ATX��;�}}�%�>�����7�y�yf��<(i�ZiSMV���3�=K��_`�_lB���ݓ�y�ˇ"m[5�쳔�Ũ���T���_�]���d��R+��(3����i�kQ�ė}���xt3��r����E�eU�����>��D��_P����c��b�$����{)�DB=i�?��>��/O��u�O�����&}gR�%�;�\���eқ�X�;r�i��+���I4�;����?{K;c�C�bU�u��?���+�a���}�o�Ϫb�-�"��	��sR����B{AB�ь|��
�[���QG�t�?��i�䴿���!N�ņO_gĬ���O���U2����ATە#�,ᖖ6��i��{Xz1,�Ka����to�ڎ\W�V@�hՀ�bk�O�O���@x����{��O&#�Tie]
%%+��n��V�D��"=&��k�;���V$#��� �Ǎ ߙ�?�gs�vV������S���Q���S�=��N�db>!L"e�DU�Jɗz�6�����]	T�,b
n�K[�mo~o�����3-�Z�K2���z䐬 %���(̾B!�!�[[;��sm�*�����1�k+������l���a�Z�x1� s� �Ü���˞	�'��N���L��~��o(k|���RN���!��Û_���k���r5]�E�<�H�`��r����v]�W�f��q�[��{�n{Qc��l�g�[갻�R��ev>]%{��R�KKB΅�v�k��'8�7V\�sG���4[��!�)_]i���*��S�.��w�U�L�����i����
���� ���� ��?���_�� �@ �@ �@ ���/N�� h 