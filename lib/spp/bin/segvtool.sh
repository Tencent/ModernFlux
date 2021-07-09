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
‹ Ëu*SÔ||Uöÿ¤BÀJ"(ÚµU£A$¼¤V´CS™°A«€²n¡Ò•G¥“ü!RÓîŽY‹ëƒõã®U?»ŠâmR
´EÀRä¡¸PeBxTŠP‘’ß9÷Î$“Ð
ÕÏïÿùüÑtæž{Ï9ßsîëÜ;wf†sNá´93çÌ›¿û¿ú7þÝ>|8½Â¿è«Õzû°¡ƒ9ë¡ƒ‡µ½}ØíÜ`ëÁÃsiƒ¹ÿÿœEÒ#óÓÒ¸ùóæI¿TîRùÿŸþ{*ÇqOœÁNÇqwq˜úàÊ,šÎRéÉ+"<YÜH®üMá®áº@:AW.‹ËŠº¶ª¢µ«Q-?~#ãXzd\VÔ5E-§]ºkB”YQ×Ó\Ô•ãÌa>ÄÚªÒ[+r£®#yFoîÍ§ò¥-gÔ´å¹Q×
XEŒ}¼ú› Ú7AµK»¦©åÒtåõ|’Ê'©åµk²V1|¹ßI3ð~Æ,=ã‰¬¨ë›jAíªñÝ|]:Ñ^4ûn›]ð§Ãn›=ãÖÙsn]0rÄ­#†*š7h•mV}>æÞ‰Q~äÕúÃëxçøüÑú1t§¼
~ä¹~©ð»^¥õ‚_ø]©óúãwð»~7ÀïŠKØÁëîãtm«›Žž?“Š©;üºFµŽë­k‹}à×Wm¯Ið»~×Â¯ŸÚÖ{^OOCt™‡Õk0†÷-µˆ¡ß¨^÷ÆÐŸUË—ÆÐOªôÔ¾~\Ö¹QõÛÅÐ¾ü;ÐûP]“³(†þ±zý0†þ¤ŠgY]ëÆ…1ôêµ †^¢Vìðº¬^§ÆÐýªÞå1tíþ•ì}§œ$†þ†Ö?;³>†~¥zmë |E½·zÍë ü]1ô~êõ_j½‹j½kýaCLùµs\Óv©×úüPC_õów1ôîº¾—ÖN¿Ô]½;(ÏM[<í™I3çgÏ~¤¨hf7mÚ£sæÍ†3½4mCWŽT#¸¢Ysò¤Ù\ÑÜÂùs¥|(X$Í—æÍž©™óç>Y3ç<:Sâ/š7_âòá¶™‘8ˆ™ò
r…3çÏŸ7Ÿ+tBFþÌyù\~ÞìyE3¹|UbžT0g&²Ìð/ÊšW(ÌÁå=27Ÿ+˜;Sšöˆ4o.y…‚‹ T‘* Hš*¸ü’ùÒL„3¯ËŸW8s.€ECT«æ<R0—›S8ž43˜ófÍœ1maÁÌÙ3 XÞ‚G@ÃÌ7Æa=mÈ !ƒ†³‘ï·ý§;tÜÖÆFçÕÝ07U­÷uÏå†Çóô³·ÚþÔ<íŸ¨£_££ïUåtÕÅø¯IGÓÑ=^GoÖÑõsA«Žgü-Bïªçtt£ŽnÖÑõsJ²ŽÞ]GOÓÑõsWºŽž¨©uô+uô‘:z½Ÿut“ÞÏ:ºYGÏÕÑ¯ÒÑ'éèú~9]Gï¥£ÏÒÑ“tôB½·Ž¾@Gï££/ÑÑûêèËtôdý8¤£÷ÓÑWèèWëè¯éè):ú¿uôßéèèè×êèëtôTý<££ëÇ­:ýz}½¸Ž•LhÉÊËøgøŸ‘´ÑXÞs©™Ý˜+à_èFLÏB–@SþÝØÓØ54‡iì?MŸ+‡4v…À4}
ÓØ¯ÑôQLcÓTÐôALc×,¡é¯1p…4Ýˆilúé4½ÓØä¹4½ÓØÔY4íÅ46ñÀ`šþÓØ´i4ý¦±IÌ4ý¦±)8š~ÓØ„Í0ý¦ÍÔ~šþ¦¯¢öÓôÓ˜îIí§é'0Ý‹ÚOÓó1Dí§é?cº7µŸ¦ÿ„é>Ô~šþ#¦ûRûiúL'Sûiz,¦ûQûiz4¦¯¦öÓô˜¾†ÚOÓC0Bí§é[0ý;j?M_ék©ý¶·“‹ä è:Üœ;Á^ëOsfqbmõ`z©-ì‘Å)gÛB¡ÓØ†D9aG™™Ý~).Ô@›P…þß’Q9Ð>¸‰"ùN2-Àã7Š$a—Y#0o…Q6‹îFçáÍ	ý ¬¡¾&??¿‚ý*&Š®Q_<P@Ÿu´é‚®D_2,ì¤¦^ª×©ÉË^™miÎ±wÉÒ*B¢)ÇzÆAfX€<ÒJI‹É€Fì½¡Ì"û}Ø •ûÀ®ÖTàj%ÝŽ *žºÖÕz·dsµÎ`xœÛÄ¼„yà2‡<°&è¯pµ9½@šBIÃj‚ï‰òPŸXœÅYýÊð“»‚2$ûz‚.å1¼Ãî¢pç §ä„¯•_å«óY_¯n=Ó”¦$j¬H6™Ê^°¢Ì‰žEðŸÍbT6 ðL
3ðARô8¾ì*+Óòfù>1ÂÅ÷Óbg9i,'sêXÎm¨ßürRiÄbn¿l4•uíòéÈáùl0^HuŽJây(Ejd7ÍÌ¨1¹7ÙAŽˆÕÁîÊ…ŸC!ò5ZØÌ4l;ÖpV)ò5©¯…¥‚NåãŸUÁ™`çƒšÐÿ¨BÏhB yŠÅ,Ê³-ÉV¿CžaIsÀ}ºC–,ƒ¡QŽ¬åZ­Ø¡¸‚÷áÝŠædÆ$Ž4ÊÛ‘ŒÑ“8D)¼»/œö±¦#aÆÍXÁÁ;ï”épçB£.<WÌ[ý-5þYøŒóOyž¦wÕE
©ëß óKo„¾øTf—¢½Ä/V7u=¶.|¦QúÜêÖêúCX^¼Õ×¢‰«ËsûüÄ}-n+ˆ«‹ˆË¯°úK/ønîÆ•\AšÕ‚#ù–¦LËoyØPb‚|Ÿex]™
¹«GMÐ™²ci’<Æ˜±ÙTVŽk"¸­6=ý¬z»Ù´ôŸÚmÑlyïuí¿&Û³ì©²k9Rwö ©«>Ú« q	$û7ˆy¢'é¶—’<‰]H-{ö(”iM­>×«ÿµà²ÅõSÈó4&L÷Û’)-MZu2å4†¹+ÈW`Î €ü 0|t!csÉ¡Œ>j3•@ÉàjOÆÞóÈW>þ%è×9??ÚÞ%¿ÎÞ»G›Âöö,hÌ‚¤fïsØÛí¥5{1¶wI‡ö.ù­ö’æñ¢¼ÈbÎwµÆ9ä	–dìhÐ¹¦XÒMk¡sI…‹§B^¼©W|pÇ;*=W»
¢À¥AJé%Þ´ô)\º6ˆ03ŽÎx¦ç@³ÇzÊzƒ»qî}")Ð-‘öåö8 ‰O®Éw)SÙÀ?™ZXSz®d;Ï•ôÉu\EÉu –l€1P¬>È‹ÕßwûïùLc½8SÙJÊ‡ó\Íäuçû}Ãâ¹’»€ý´!¿¢d8…µŠÌgîpìHäœ7ˆž²;&À†
ëi–¯5T“×µ¡°üŠÒsKËâ¸’¬|×…!èµ’¤Q‘ÕMF¨Ï
@7°gwÎ™
fÅƒ;&TŽéóÄfUÔ§¹ZÊ„é¤Q½Â¥ ÓG×O¾+'F¬#i•¬‹¯J¼”y,åL.½@k£ü%ð-sºTêO½YQSzÝYœN”iô©¼ƒMjdé4±¸˜!„a Ü]ÝÔú»ÃXO"gË\Žîv®Ï‘EÞ÷˜íŠ]‚«Ú`ƒF~„6ñ Aè¿#'Ã(­¨-Å¢øO³x ¨@¤%F,nõ‡úïˆÈkú‚;ù§EÏF[æKèQ“{6À±Év^X÷Ø•{†¨…³MÕGMË 2¨´et3•eCÁÚ¥ÈDwjžH¥FùqÞµÙ@š•ª,ýw“Ç©Î?6	0Î˜¹[…É}”Ì¾áKÏw\~ÐæGû®,hÌ	ýr2 ¥ÖA±ÚRda6
`£]µ±4l£Ê„6ÚU}Ø3@_rÆrì&w.Õ—ÍûÜ»3éÛP}4ž4 Ò®uP«T)´PU:Ð§*ý=(ÍF¥d°€f—ùP³Ê‰šÏ×`û²'ÃâAù¥¡:îƒ¸âáæ´áƒøâ¸Qâ?à‹{ÁMC×Š»ÃéÕÏïòwUÇ›¼Ô-Û†PÚª@´S×R“ãö/ÎÊ±6f®¥á€çU¼•¨È™ø{Oì G(èYTDF!ïðLâ%“àÚ`ÈÉhp~Íú-„†R0ÇHâ¬~Ñ]/½‰Db ]æ'€´ ¡<[­ôu5ZjtÆk“ÜÇêÏØáP[ yS´þ]¾+`X0†Ôå€s”TO’qÇ9TeÛR>c³óúà`ÏØQ÷Ô‚‡7b¤6DJ4KæŒÍó¿±yF Ðâ}€‰:3"_t‡¤A¢ûŒÔ_î
îè"Ýü ŸÑPÜ·Ã¨Õèù^ä©¥Û0zÓ\©…‡„…‰Â;9?q<F¯"ÁAÛ4k#qX’:H¢Å&?•ìç™ÅZ›…>Ù’Ç¤¸ý¦²÷pô"?CÎë@ºù{C…œ&@?]5|N†dI±ä¦rÜ~L«jí†M²Í’dmÄI íìÇ(}Ý3»7H=!„ž·dACHž#ºxç{@ÆÒÉ‘<çßE×NJ`IY·mëÖ­ÕãÎ¬>—9Á2°8côÁt½'’£(ÇÈI&QvXŒXå¦üÄ WÑd›Àâ¹ÉS…)ÂTáaa¶5ÖÆàQN´ÔÒMLØl>û­þGþ+¿‹Ü¸»°ê™÷ð Ë´ÆÝÞ„‡ìä‹è*õU†Õev@Åå1Épgt•Ý—â®7•Õà­ÃrgÕ&{V0Å_í×nƒ›Vx¬À¿a­9H+­C‘ì†Ú<{F4­š`Éj§ïQëñ­vêq¹Z7—¨ÈG#i¦)YÌáŠì¢¯H±½Šçèk²«Ép¢cd†˜&ÁØÆfÓsX¬ã'Ÿ“Àéf2ü¿Íé½þ:<6K"é½n«¦¼˜ËysáOmÂÊ…lGJyéAŽ»bŠ%ÑË¼›ð,d Ä Ð<9º~Ç“ÈN¯ÿï¹S2åI™õr.ÓQ›P·H•™#óÝEÑ25{`½ÚŒvÂ:µDB;i³†çIXÏÊhXl‚VúÔýnq—Zº¿(çp™ñ¸Ð1c$f‚ÄdÖq¼Àf'’³U2¬E ù•{[N™åmÃòÜIº®ƒU¥sxàu XÏx_¤(ÿ<I+®JKï?AËJ ?aõdŽ»ƒ·8ïaq82€¾Q‡Yì°&å+—$gq^ºìÉ@&(c€+ÔzéFD€õ\#zž§8¬þ*]¸QVB!Aæ-£^-7p%7	.Þb d†W ¶çùà³¾'q]5°Ñª®¤Cð­üH³“ÝÂD;ùÑA¶&‚±ãA„j³Ì²A/K„ÀÔš]Ï´ô'tb_ÆÀ_ú$O£›x2‘ÏœÈK¹ž±Ò`“g¸,|ÑƒBéy,I¶q\I-ó­t(Š|ö¼­"©cñ¿Ì&?ŸŽFŽ»ÑùfŽûðÂëaeéQÈÊ"bl ­€‘Ð<`7­ õIe¤,ƒ`'Nô|: £ÅÒƒçCñœ2â8„è´t¥é˜‰¡£’z¾6Ë‚üZÎÀá]¡¤ÉØØË hé7†ûXM~)tÉZ@Ò‚¹<ŽÀ4 ä2Ž2ÇñÒRS³—†qòlê‡ƒº¨¬´Ëè…Bæ{
DsE•b^›HÎ‰0ÆÐ(¢Ãþ÷ÈÏ¤c¬—ã®w®p7‘­T§9F¡Sù»”ÒG(]ÌcÔ‰ òÒxZ^kÉ³ËUá°©´Ë“-éQR-d¾/B¼U´NÌÛ&Tcàv\6c0ˆCœ¯£îTS9–Å\:•É}…Ò§xQ÷C û!^ºŸêµKë0ø¢AªÇN¥mÈ‚êM%UBæªtˆ¼Š>aêé
Gþt$FcîóU÷ñ…×	òR,2Š­‹ðY ÙäÉ1 ƒ@hŽXðòÑ:ëù4EØ´úêëà
IIFêŸgõOÒ±úG“²LÔß~ý·LååÐ…hVgå?B'ú*ØÞÑÍ(XiwÓq« ÛB+‘!‚:'?ÓmêCPí°„ùt .atðKŽÆÀ€keúÑü¸ø°å\?â5Z“à5¬lÍkCqÑ¦SËÅªŠ+8%ˆ¨Q;ÍÅ^‹,¹p¼Vp¼†AûgÜRlÅA;ßµt	>0:3†7°é©¯(§Xp˜6ân”Çh*Æö¿ƒ~²iÀ»eÀájëµxŠÉ÷·%4ä¨>—z¶ÂÓÅv×ïyCÕ;P¾÷pì»ç¨¶'2ùd,-÷I­@”*?«³w(²‚£npI¾¶˜5þ&Ä¯à`9ÃÒì sOÞ0c–@ÎÛÉ6ÅÀÁÔ,dm´ž!µÊ£è1Ü%©Î;"š–‹¦MÂ¶é`ß¬°‘ƒUß°‘ "@šÅ ÕúN£-d©JÏlÄ ~„¨}K~$“ €Q+ˆdA:LQøô |£¨P†!Œ,„!"Œ\@0é"'†àû#TçV†`/KU¡‹¨ùŽ¼¿P¯ ŠÒc{éNæ	òãæÓ²Í81·äªXèé:e¬Â%9 ¼I‘;g`ólÏØ$Ý&z¦@CÛP5kAG01B©{)¾Ê= žî|úæz _ïPq“(¿bÁoQ^iÁ’C~ÓRG¯+-H/ÝˆkpS¸Ð¾ž=lNË0q>4v€I…›m–Eœòý÷šüy(¿äÿI¿H€zÌêÏ±F0•G¨)à„ÆC½™Ê«HÉ£Í)*~ÈÝè]c –0†©*»ºúá•j´‘ÃI7x‘-øn­Â°`†u
ïˆr4æi:æ>:æ³€ƒâÁ*ß9¬Ñ>G´Øà=ýæ&ü_U1ÒÀwªRLßI/ÁôˆpšœŠDdK¾oS­!-¾ñ©€kÖË+8¥žmŒK½×E6œ©sveX¾£LäkÆö\›jL­Ô„œ^¤ZÏ(·„ùô¯›—`l1­jðxo½øOUq×¬å”?Ý~”DomœLW)QöR~u|‚ÞRhçb±%„†žw2™æìÆ` Äi?göô³Þ‚ö\4pélúÌAVà«Vù¾G¾Ú,ú”»–Æ˜v	ÆÈ-Ý.÷¯Žý;`U-ƒ‚JÁùËõ/`AÊ«QÙ!Ã¡‚Å^ÍEê—ªéüÜYýÈ«|ôóoÒŸ‹úETÐýÈAy•!¿MÿDÔo~F‡À¹NèGÊ«|t®#ý"¬YÔæi
ÿ¾Ds„ýñ‡sõò*×Ÿ»¨C>h'»è@‡\<¸.Xí¤­øZ*;¢{E2Îyi&ÔØ<oƒ@ W5¿làl0Ùüç'µ£×HF+ÌÙS¹ ÿì&<5"}k+­ØÓ‘b¯x‘ÈÆ¶k‚%u‘ý÷S‘ˆƒödÕIÐ•É¾ûÐIÈ«‹Hƒ~œÒ~?f(4wíWÁP)JKkX@“Î3ßBÉ÷õã= ñ¼ß-G™)l¶l%; TUæ[­þÕÚÙQ…ñ=¦áØ ¢­9kHæÈh^Xè3B¨ÜÛ¡Tv"Tè«ù—oÄ\Ôßi|Œïó³À÷þX|´©	^`»<ï½•Ê€²vG¦«@g0‡ü‘)”R5 ´DÜ}ä$ F~%ålxÖ¡p½H…Yg5 C´6¡nð˜öÖN|›ZCÐóß™ËžÿÞ`óe{îLÌü‡TœÿÐhlÝ%ô¿Öo¿|ýoêô÷ì@ÿ;û/K¿õoœ
=¥þÇËÖocú)Ûs?ÆèG*Úyú³Q¿ñId¿|ýÙL?eë«©hÿ/Kÿh:?@PêOwf~ Ê«<w:Üqš"C²ÚqÔ)‚núÉS9ÅùÖq^‹~ÞÉÎðò†j Yœ¬Õõd¤lF<8úòªFfD°pK>08Ô†ûïKÂÝ¼Oƒ[uèO!ìÏ…-õ'ò*£[:áÏ÷uàÏKÇ‹¤ÒxqïºŽÅ‹õ§:9r"+ð-?žay‘Æf˜[öaûÃ9+¸úrðañëÏdOFgñ +îOêñ áyùëÎá±2<¯¹#žáùÏÄƒ¬À÷ø:<HcxºwÏý¬¾R•ªúu²ß·Í:<HcxžøJ§Ãö= ãù= ¡þ½¹í9(¯ò§f]@‚d¦þç½táˆQ®—*cý±#Ö·©k¨ÛLÌÝš;éd¾lãÜY<3‘–ªôËîHœVÅ}fHƒ„ ¾fúÉÉÎêIÀw{ú©.Pác öD\ºI‹ªúËmoóªýÕ¬hÿ‰vô1Rh§EN`-rër©+C°ôD' +ðÝ‹ ôWjfÐ¼Ãã½E·bN;ñ+f¿÷"ü±øû«+fË+bøkw6vVà{öxçWÌ1ó‰°þ"¼;;Â»éêqoßNãVà;pì7ãíp|À…ZÕJtô…Ç:1> åUFûMëÝ1aý?;«y•ÁË^_’]4º÷ä„Øn+Ýk„@Ö—9=êB(d*INùz]gàÈ	€©Ì†¿a¸‚ü…¸ŸìöõZ@·€wj»`ÂÙú¥ÞÜæ;Aå+Ý#¢’£lP}8ƒ/ý¢¿rÐ_Ü¿¾½J©?Ú	!åUžclÎúà·µŽÖ·Cqi¸PO8ú«Ö·ÈK¥(½Žê¦$3ËW6†B¾ÁJ#Üßüe<Yè<8«lü*<ÈK¥(®€’žÄs'â­ÃÓN}”#~õ6AÉt¢>ƒò*q‹Ú/4Üê˜þ«Ûßm•lQ«a8ØˆÉ	ˆ˜ÉçÊ*%¼TÍá.w©ú5åVŠT^yÙ~¨OßèþL³@¾„u3°—ê¦Ç»Ã(.ØÈ÷6XønSÒ•°k¯·“kc«q$ãÉ`&º’¾$G™åà‘°”Ãö¼/Yõ<Ù eÞÕŸÇ}È@šé¹¦|×¹!ÅƒóKÏÕqÆß7§Æ_7Jü‡|qo¸ièúaBq"Ü^ýê]Õ]ó+LkýÕ¦WÃI&Zý©??¾²+;¶ƒgì[ÄwO;÷[’(ÔãY_!£®¨'[ð¸6'OUxJY¦¿èøû×É|¹Û¿`Of\ÑNí$ðì8Þ–™Ã9aPˆ—sxÑS”$zžL&Ù¼ìÆÇ$Û(çŒ„Õ‚ì® I³imÎ|’}5\‡Á5®‹áz­i­{+ZN²›Ö¸·ãë2ÙC”?lÇ“èO¦[ë½Ýéñåö/B!(”|#à:
¿F€»ôîvX±ÀZ,;®f¸¦[ýôÏDì mPÉt‘Ž(ôíá{ÖbzÙY‘6>a5½¬0ØAçêê\æõ‰—Î}ÇÆ¥J'·‹ò“œ2s[(ÄÒUÑë¥žrö(ŠçC&Hº¡ 

‹uÑ.ÛéÓ µªÄMQÑ©òš#q×³VYKA5lÕÆ÷µÞLú.Üš­^vÆ>ÅB³ðQOþ6Õ_÷†»
iˆÇ¤[¢67K¼J·a8DÿÁ|÷‘]¾Ó¢ôÛªùò½ÿ²Uÿá6õßã‡ÛóÝ»Ø¼•úo´Î·©ûTŽ2\åí|úÍm[qC ùs\uäÒÏUGæWWvT¿oªøp›ˆâÛq¨=|toé¦öña•£¼p(Ÿºùtk»øŽlé_ç÷¯VUÇ .›nÎûœÂþc‡5ë/CÕÍD¶í –‚ôÛ‰3?o'b_fæ¼M@²fNðEÕ¿¸£Áêÿ`{þ¥[$›·¨@#þµ2ü`x3dÐ%7CoÔïPê÷yõa áõÅCvrÖ^}$-úL_:é½æ3G†¯„¿"I´Ô&<ý ;ùEkæ«÷ò˜d{·=ÄfI‘§¥à‹)óÌî}¦²Ï1K²¤DÎ÷™Õó}‰ì´ÌM¿p¾/ÑTþªz¾/E¾ÏWñB.u¾/­£sšfv¾ÏWÑçû:8§™1Á’X4[90+ÄP˜E÷qÑ”³I$	ÎûÍôØ„îÐ-<$ÊÃ,µ]´Ó|æÀ€ðãcõ¼¤@ÎÆúvø™ûñèÞiŠG%kªïWÛÛwìè¤Ež§ùô¸©lmÔ1×dÕŸIì˜k{^L2•ÿ]õ¢¼hm´¿˜Ë±S®è>KGÇ#“9çJtŸùÅ(÷­øÅã‘IÅÔÉ¹ªÿ’#þÛ•Û®ÿ’Ñ6p §90™¾Õ³ÿ =ä «B¶
Î,Éó¼¯ßö„îÛã ëb·F´‹åÕjÚ[J}#O@ê÷k}Íi¬`ÝeË%Ÿ„¼¹™ö9:8:*ÄmÖ:Û3—³^lÝÍUe½‚ûßû;ï"åUzî×Å»,ÀÜøúB¨ï5\~õßNèCÊ«¼ñßöô‰ÚVTÖ’q¨î””
·ñ’°ä¨z~è®5ëO…îãè‹*±­Ž­Ê¿FÝÁÞNÇ‹õ-P.¸{ýxù‚¦P®BRÍbFw½ô‘Œã¼˜‰3÷Êë›èƒ€ò¨ý]ûj	·/„L²0ÐðWE-´Óz>;Þ`È¹YsqcqÙ7ùÕl;"¸å³(Ü‡Š?ûopµü¤o¢¿l‡×‹™¸²‘M>Ë&ñ5.ŒêÖï¬_|kLùi_'ê9(¯²a_{õë ‹,­9äÔÚ€Z÷¢óš„Ò#¡¶]þ+¸lifçÂa ¶“Óv²«ªnÕÆØÈ9ßÒTÍ¶ÓSý6XËLÙ§EY¦²ítƒQuœû°H’¥+D‹_r_Otã8÷qšÎ¨$á
jœûŒÔÒŸ>¬Ül']À,64Á}®jeÉãWú–á¼Ùöu8ˆ»•Ž4 —¹ ]†Õ˜ŽñÀ`\0tÈ‹,Yxl[F¤­úZ"™…„b8ïu6cÈ#•n´Žÿ’ºöJdÁ¶ß $¢3¨'”©‰û¼HfÏ€ˆÀB¡
}ô»)ü@§2ÿ;²%¥½´IbÛŒ>ŒÿãP¤êxvyê:
‡‘YªÃ˜€å¡-Š@ó¤‘e=Vî®fÑ›2n‚+"7aˆpÓÜdHJ˜…úõìiõýdXÏÂ"š.iíd7¶zŠ,Çê÷á{è•i¯wÅcÇux<ð‚»Qº_\ª«VRu;Z36/>dÏó»Nð!’èöÃ”î
Üõ‹¯p‡w¥¢ C!¹
ÅùpnTž¿g,²‰æÎ4úVz)Åýƒ±ÎGb³ê¦ý—lÆ…ú–½Z•8ûÓnÊÖï‡É^<-½'æíb•e¯ÆÅ…õŒP™L?Cu}å 'é4í¿ß*‡†ððÆˆ7µ¼‘*©¤ßB3-ûCÅ@»6•Ý·Þwßyç¢àäÊ¤µ102¤¶{¹çs9u¿zz<^§¨ÂÈ@ùëu ¿†¶>÷>g?ôBºÈZû4v”ùûÑÎÏ÷hÇç¥ÑšE{E@5«–hV!jÞ€UT©”V·¦ºšÚ‚e¤‚ç£`²‰žm1¢¡aBÞNtâI…÷C{V?{§£plIxâßuld¾ÄïÃRY“¦·ëšHíÒº¥/D«í_µÌ‡ß7SNîÖÛ–cÛXõSm›®Ù6Ù†Ú•9iaÛþ¢ÚÆD?¿;ÆºYzën‚)3èÉWç:d*ùÒÁúµ„O8~TûÇ¬Eoè¤UèÃOS(›Sõö^co»my¿¯°*çw…M¶	•³Â&ÛÈA9á g©Ñi¿SnÒŒnÒUè‚Ô°ÑnÍh*üÕ]1Fçé¶VÑr»ûå­çÓÙz¾çu˜ÑŸ¶]á19ŒÚ&¤ö<}	^·ÁE‡jd¥B~—n¨F2ë«ÿø0­b£óÇù—Äsâ±ýÞÁ)«¿ü5x•
Q¤/uxÌð\y™xÔýŽf:©cX¾üUû³ÈK¥(‡vêög—pÚþì¢O£ÏCý2žáþ‹cŽ·;ä¥R”ñz<HfxŽ­ÆÃ¦@„DßbÛ$ØH³t×¦4HÓÂ§ÚbP†¦µ‘'•ó^z{läö×³SÏ8ÜÀT/…BdoðŒßhüéûãOD\1ÒréaZ®¤’êNi‹ëÙ)í,¯¨gŒ;¶8Åý:4–=œmÀ])°a$³aHÄ†½z0j*©«î[j`úF%• ÇúŠ—a}¾RÃúDÌz,|þ,ä Zw8îíJŠÈAÄë¥1DVŠ¨=E÷>oTÄ]Ië¶T2 ÁuZ´õ	Û_nÄÓÙávE_PTú0<BTÁÓ‹áy}ÅC_Ú‹@8‰â‘ˆoLÔT¢`/mö(³T(ÓÃP¼‘ý6†'|Lˆá)_‹çðº‹ð\u9xP°wt4ž½ëž†µãÑý1ê<è,¶èi“n¥èQ\ôHá6Øî¸¿ñ). -âr§ùw×RÓŽ˜ÖMgÚõÌ´kic|ùéÚ¨JøFZZÆ°dÝñ/•úø¸¨—æÜÀøSÉÔéß…8¸–Yýõ­ ÷×™ÿÃãó—µ±þŸ±ö"ÿ/_sþGÁ^:,GLR‘ä®¹Øÿø_›0½8k“oœmærÜ‡Mî—hD|¸6áA ÐÕÖ]ØžKkp‹ƒ-/éþÄ±„ÿnÉ‹¹œrÝv:bšÊðRÓVärv×FƒÝu~ˆ4ÖF©C„ç]/ [šÖP³'GÌž³F}Ý¤<7¶–É¦Ï½UIÎ5‚œÃyÏ–¥w­Vº`<jr³šÊ+µ×â#¯¹ÖäW\<¢Œ]Ã¼1ú“PH/?øOææ[béålqÌ¬U2·…w>34#£vbÆÊ*‘íÆŒ•ShRéù	Ý‘	½Àä#‘~æë+|ë"ï(ÌOÇ‘&âŽåP¾CÊþÕ¡@r²älc%~ D'B¼”13ñÉY‰ù¾KÎæÙaDÍe&fy¼‘äò‚iíDK>É5B¶§Ð@šÏ~ùâËnRÄËñòx¾ƒç¤Ädã×>8®¤ßëP ÄÞoÙÕŸãp”mT–ã™!àI\&†ÿyÆ„*±R\­=J’È¨á£qw(üµÖ“Þx%uQ¯TØÉŽZúU¹w7ÈÍÁ£G­°›ã±õ/Õ‰ž${-ßÖàä„øm/ñÖÆàÍÚóSowÙ–Ò¼t7®ÑD×†DõS	Òí24¬¯¬þB¿JQÈKýX)¸ÔU²¿üŠn3`IuÁœ_ûþ‡XK¿&;äÞýü4Õ•· ¸ŸÀ k(˜ì0Ôe¥;EW­Á^êG%¢+”.Õ–™w~æ_˜Å‘F‡¿/C?Æ9Ö“dÎìîÜ$æq?¡FÎbW
…½<_ƒï›ÕÄìi®pÙ±¤InhD§%«˜wFyå#5¾`¯Tµ}¤ö·œöæ=4[ë½+`¸«ÂÞP†þsËG¬ÿÄ}ÄÞ2Ë!M$àU Pð+Öƒ”i}5ˆ}³lTï,üfYH2+ÿ™bm—eVŸº2„ó¦±¼cw«yøu³‹žkû<6RC·zÚËu¾B€ÒcXo‚«uˆ€å»ídÛ8²•ú­x$¾äµÌÀ^òÂ<ãK^+ì%¯×èuµåßôú©÷¥ÅÒ(Ü©_’Æñóô5‹,f¹² WÙÈ=Ù¬ízp¬Í’¼žî¯Øóü´u‘E!æ¬6ˆ¥›Yíÿ”.M¶6Â‚z˜‡U=´ì>vÏ{ä®0áÔÉ]369·;È·6r
÷ÃwÁÀúW¨µá›“ãÈ·\3Â%SH9]Ž>½¥+ú?XÏ‘•–B g#«-ZR[f™®~csñ$‡çyšëð<C©¢?
ö”›¤[¡¡Š®ÍF[F­Ô×.÷´ËQîÑ,š>¼Ê¹:ùPD¿VX	w60 ÑNÅªš´ÆÍ$Ñu’KOP{KCø5AésjòÚ)OP‹§ìžÄ="È&e–\üæY™e‚ní×ˆ2#‰2ÃÉt¯Ýw\Ð¿_)wÅÏ³)è…vW!£ÙÙ_Bß‹;`
™`ùF”'Xö¢€¾Î«Ø7MŒõ~Ó¤¥f]œ¾ÿ¶/ï6*HÇp÷•|9RÛyÕz×+Â§tNMP&¿ÝIiv˜>¼ÁùCo±êž—Z·îÃ^ìŸÏÆ»8ßÐù\›ðqK¼£}!Ýt'È-m£ÃYQNX}'t2OÒ?èÐš¸Â]ïÜ„Ý‹ú.6Q$?ŽgÇ D×¨'‹3•}IÇã…(B¾òÏp±†ÜÆÔIN6*7ƒäâZ~9•§Þ‰£òr•S_±ËVj4dNéÎK÷Ñfµ!¿sÀ¦Ûd¾;¾2þU-ýh8Ûý‹:B9‰—Ò(—_ãºÅ&ñ÷ÃÁz€³/PåÕBX…DMø0S“àë]K™®P`Ž—½e_iÄq•4–	NƒÉ¬áe˜§Ï}ƒ#$-d"]°‰?¦{„6Qïž‡3©{þ—½oŒª¸ú¿»Ù„$ÀnPÔ´¦z‹Ík“°I€Ø»°Fˆ&ˆV0	É†lM²1»!A#…nˆn“`ê£ZË¿âãóóSª´¥¢–J¢¨‘újå«ÔZÿ	 Òj‘Zd¿ß™;w÷îÍ&Z,¶gçœ9ó8sæÌ{îÝ¤¹Ó¾Ïš#ÞËºíÖãŽÎsfS¾î•ûäqkØö˜á»²€rˆäÑuÎøE¦Qã'â—Ï‹V6>?;Ÿ¡'?×š“w­¦_ò^jôGÓ„]«êà»p6]¥ÿ¿ÓYe¡7˜uBêC*dÏô`…$‡×GsX}˜Úoa&	uòNóˆ~åt&\¹y)VË{cýŽ¨èë)ækÿsÊ(#½þoÇò-cª·ÂëmëU?8ø"žŠØ_·:6¨Š¾‚¯ƒRµòBV‡_öÎ*>=ïpâ7ä·oýÚ
vïÃÇ1´ˆþ CÔƒ{|«ÈPÈ[#ƒ^•SÕ²JVÕï×ß¡ú5ùç§£E¶2Çš½è¡÷o b½1òè•»:ŸUîZfƒÆ{ó)3?”3óŽ¦Zåkªeç8ò©rP5köšÚŸc•3´µdä+gªÜžCõcj¿ƒî¬‡×Ñ“¬ŽÆÅSxiý6oä:z6O®#æù	^ó¡Ý_kjï†ùÐR,›ä°—C"4»éöÎ„B6,ú>R­V¥Ú·_o×UÇÆÒ
ütÄæÐÆæ+°çQ­^Y]šG‚\%²·ù"
 ¿t†!ƒoëTj°›£ZMÈZ«åÃÕªý7r­^y<t`¬GÛ^ŸÈeUz;k¯å†µ×;s‡m¯í¹J{mÈUÚkun°½^›;l{½,W£i¹ªözß0í5)w¸öêÑèÂÍÞ9¢ßoY6Z•Öúˆõ¤ZkK(b]xk­³R½~E-Å¯Î{™Þi¾0¬­æSþáQªÓ™VªÓþ°:m®Nß’ë”^f£Ñ$«Ï_Qy”.0¦gàüÿ¦þ“õÃi»¦%Íò½Ï÷ÂÖÊé~1¿z4ÛáîFýc¢+÷«Ñ¹
Ð·ö|ùõÐ¸»¢§…Æ¢(ß'ÆÝýip•pÁ³„–W	w²L}ãB0ìQñ°ô®ñLSñÜ!ó|’“ °=U$þNŽœø®èßãÖ7eÊî e—B)dç—TïÑÖ«¯àA
I;°Þ©kß mºZvcÔúÌáÅ•1qðGÈÝÀhP×íÎ¡Å€^ïÄPpe†¸qÏ¡¸›=¸?Ò}1‡êÂÂžEam´JëMGFðùÃN†z(7Òž›ñ”›X–›ñ¿Ï¦Üìö&Ã³Ì„FJHeC6åèþƒûBçÁ5òø^¶Z·d#`:?A:¾ œSÎr°wXy¤gGG¼¦5àÿ¢X¹Eðvp/Òw(ou`‘žö ßXiXs¤yò³Ê
—¼Œ/oy]™xÓ²Wž‚í€R}ú0{J™¶­¤SRþü»jvÌ‚P³ËnÖC? BÂÀS±–uÛf¦¦¹³¨WÀ¼æ#²'ÆÕ=¸Ph9¯°ÛnŽõ¿ú4ùSÚýÛùýž+ÂÒm“Ó5µÓâVÄTu‚’ªô?˜8˜\oÆ,ê2Ã¬nƒÎîÏô²?Æï6Óç~Óþ1˜.Ç<öíCª’Œ{˜,Ñ×Ÿ0Ñ—¤ÐƒÑ3Ð×‘!šÌi¹lxè÷çÈð<J†ç³½l’\Ñ”YdÑ¬ù#=Ö½WzJ'g¢ý!YÞ»LíóT™ˆ˜ÇƒJŠônäÖ"ÊÅŽéQåÁ³wð‘šSÐ—IO‡ëËpÚòv˜w>Ô–kO¬-·= kËÇ²¶Ø2™HŠü2µfÏ_ÆqIdã¨ú«¢2EUý¤2óÂ/z«ŒíÄ*3þeñ”¿JGªÂt¦¨êuÓË6¿ÚQ+ÌŠ4
ó6þ>K¯ï¯Hô5µÊ¼1Ñ{×ÓúÅàUæ¼`.î$n¹ÿ€Êl%•ðóÛU“þEþ>ô"fÀ†ÿ1X×o¯§…ÛÁE¬®Y–ä·¼¥‚:d=‘Ö+YH8®=&ïß²ù¥²…ašäX}(ž*[ý9I 4EþWL·n`¯ºCGÙ:Æ&3&¬gÕÜþ(­y­y¿sC›ÖÒÛ†ÏÒOê§þÿÀCKM5;ºëÌãŠýŠýGìtÈàcÓ6Óù MµÙ0ð§ûyÝö4z¦gë‚Ê-‚çÔÎg¯¡¯3'°Ô6˜iFdpƒ9…ÁMæL–í”ûí0ÔÕ)	Âs½ò½›Öû•uì§@™—B£³ß>ç#Q/	’~RH´(§<YâàÍ¬ÍŒ7þ,}
@V–:Šáñ+®„wñ¢tgyƒ®½njg/÷ÑÒÉÀÞ³ã*W®y‡Þ¬¬¦Ç‰hKë|ÝÁê”‘d—¸L/˜^"aÌˆß@òŠÐñgÕµ'Pš)TÆrW(9<ˆáÁåS¨SŠ¸$HûÊ—1ehù^úJˆ;i}­âxuË×0>Ç²úäëÓEé1Ä4äÿ©ùÓ“»>Ñ¬?°¢‡mÀÚ©/¤§ªø1&»YlNš¥*Ía*ÍÑíŒŸ\ÿ¢X‡ÿV:Åé}Ó±f·Ã¿ Áû2úv»A¢w&Xvy’÷—6ßßª[~aëÔ¯éõÏ½wFáÇ­´*ç½¿¨»	=ÅÞBËû–-»m¾º¨K
óÿàÍœÕiŸ€ÙÎŸÔžo6µÓs\ö¯ÝËþöüMìÍ¤>ÆÌ/²*¶›ž|ÙÒ[˜ßgj/¦jŸ¶¿ÈÞƒ¥ÇÙ	ŸIøsÌs“”`QéKƒ";¿LOÎê,{H¤ýŠüÙIËmvÿQÓ“Ûø‘×ÃŽÎ™æYŸmzHyÙu^wê%˜;ù™Öçh¤éð¨N¦Z><x@©…ü–$›ÿ5{çÜÄz‘Þ8¿ìVqù_0£è\”à=Ïö¢MOÎaÝ6TäA ‘VŸC7äý"[Y¡å›ÿÏöÎ(ÌÜ6vø_êŒï\”„~£|ÍîÎ{(¯++mþ_û[öÚ>ûÄr„˜LO¾VÔmH8xIç]›e©îÔÙóãÍ£¤Ð8,{‹t½v¿í³O)ì¼nƒÞá7|bómG°ò¤Nûf±e‡Ý¿Môm6MdEþ%-a³=t^ù-ù¼²"ÿ:¯H•Ø±½N½³E> ¼ ©¸sY=éºZ>ÐÑÅAèLþ;Ø‘cöÄêg›ßÈVFfdíÍùÝ©ÉÅ¡:°wÌÞKºueCŠÏS5¶<ÃÎÓÙýìØ0ä©ª{>Ò^y­ÆòõÞbê¶ƒ¿*¤×ÿmòë™/_Zÿ+Q°<-ryŽ|¥Ê“'—'…Ê#RyéýÝ^¨*Qçw’¨û±P)ÁBA"êRÁÒP™€\(Q.TÞÐBåÈ…J¾P£|ÿY>Ÿ£¼%œ"°5™¹I…ÔöOÆZAòÁ‹å÷bÑH:g‹Á“ñ…þÏL}ü\|N¤çŸé`|·ªsñ×ÑÓ¡ù‰ÏßÇÎÃwÏúÂžÔ{C÷¬ãÛºgÈj§ÀcZCËcr¯S Þ ·ˆ8xÝ¨…N‰ü(´£•bôcŒ&5ß³y]Œ}ÆÜ¤"ÿ¯K;Ë;Ïò@zgÀ¦5±ì¾Îáž6/&Û–ä Ý´Ýþ·mÌ¾ÙD²âEºm…ù/4ð÷›žÜîð÷ë^óîömÓÙügwÆ°h~i“ßñõÅè>ÙÓ£íÏB[HÅÝ“~~‚ÚG*ò	í6‘*ØüŒ ,? ûvÑ\4uGQß±¨+Ö¼ï½Àò!fô=¤‰Ê˜(8õ[”¨y Ò•¡§Ð_¹Ÿ—¿§Ó=˜” @PÇQŽÎË‘Á»C;ªôaŸRþ! ØóéÃÑ ºîwýŽNßáz&ù`,!›?dKô­ž…ìºO_ÅX²}à­{í‘¿[©ã_[[è©\æœ.NöˆiÕbu¥·²ÆUçÓ<¢Ç[Yuyä×ýÑgìD;ˆ^·8øé“=é¢Œ.¥°*<XXÈ«*y|fëd‹5Ýb­kMýäQ“YOPNˆ'AÙ-m®ªJK]7!ÁêËgÅ£$Èw­XãnkuéBµ§V˜ëôŠ¥…s¯KkëÅBz½1²F™JšÜU,³-K…ÎuÊdÏT9`|xšj}ÆOd‚’¿"8¹ÿ‹¤UÓär6T‹u.W¦Å•—
ÞúÆŒ*wsƒ·¼ª¾:½Î½á!ôj^èj Ÿg•»¡Ú“Ï¿7(Þà\!NžÔš*zPH„à0¤"-t5Pj*‘‡ê‹DúDàEB³‹¤!67\]Yçª†Tè“„NÊ•}ieuýìújÛÛì	¢=+<®†·`«[ê)[Ñè¤Œ”:®ë]$TUKßoå_²²\¥@²EÕ³škÒA(÷8—ynšŽ¢_T€ü±ŠâyGy&WO]—£Õî†)^ñ&g“;%sª¢u Ë#N¾©:U¬s7,[æl½µ•¢uÚ´lëEÁOì©ã*½Þ&&¶å•uÍ¤µH3Gt7{EwH¥jZ!dküY¿Eã§Ø!†e•uJ…¹XÖBx¼ME%”Ûù‹‹S¹,âWÃr´XTBí'äwÙÍ"ûtE$ÿ>€±ú#Ü»poÁ½	÷\?Ü+p/Á‘AÛAkŠp[àžû9ÜF¸ŸÀýÜ#pÃ= ÷c¸ûàî»î¸Ûáºà:ánƒû.\\}PÎw#\\=}an)Ò÷‚îü¨ë‚p'}Ãð®@àSü^£81Z¦Î„‰±.\Øv½	‚ú½7è"ü‡Ýl¡ð˜–¼Ná Ð “•„	MàLÀÀu€Å€û éCÇI˜®\¸	°pà!Ì­c¿ÁX8pàbÂ?´|˜èû Sÿ'8Fá “0U›ùø Û`Òð¦ÞEtÀ€Å€; ÷ î÷ÈpàÄ8„rÜ h'ÿ“ˆp pæFÈpàÀºŸ¢€ã~yà8¨MñÏà@;ù7!òzÉÿÄxðaÀuO!_€37#_ð í¹$E8À™€mÏ Àc€^ò?‹ò‘pù‰ú!?à§€3· ?˜ÞL\÷+Ô`Òs¨ò¶oEý nÜDþ^Ä¸ðSÀÅ}ˆg<ü€©€mÛàÀjÂoÚëv <€Ç w nx>x°x'êÙˆ| š7ì‚\ ¿ ~À-»‘>à±‘.àž—.àÌ=H×„ð€tùañË`Ò+` ðØ«à§pýÐSÀq{!GÀC€Ç ÷ürL |ü€‹ß ?à!@ú4AÛ›à÷ø÷îìzü€©¿ÿäp&`ñoÁ8óðî\˜ú;”p1à>Àqï¢€I€ãÎB|€fÀu€3S÷#À6@/ÑßC<€û 7ýÐ/¢"<à¸³QÞ÷Q€{>@= nê Û@¯ ×„~&B~ˆø>`ê‡(àâQž‰HïOÈà!Œöª÷|‚z<¸ŽðŸ"€mAúD4œƒz>‚r Îüùwz¸p`ñçw.ä˜¸°pñßPNÀ€[¡o“ênºJÐµ&èÎ7&–Žû$ñïB:lÅfLèÐÛ‰EFæÕòEiøè{Œ=zv<+ÊsEGôZ}±Qê2ßLöÁsI(TÙsÂ—Ñ÷W·kðuôRàÏÓàé¡ò^àÏ×ài?ðïiðôQ•þ³†âéfïÙ’p¾g|zÿ>}ßõIø¦><ü1ÊÏy’0CÏDðïþ>øŠD)ìû°„·OŸ§Ð–k1ðë_¤ÁÓá$ºæ¢N7®x	ø¢Ï2&Ì2&Î2Š³Œ)]ÑÝ†µQ·£Ú2mÆ<»±Ñ7¦#&ßJÿëCË±xº 5I“Î§t¼øE<Ûõ…á))é¬Òbå‡üJ’¤°ïËÒo;ðÀ—rýòéFQE¯½ô¹AúaôvÐéÌåì`‡Þ57H§UÄ‡)~Ð	r~» ‚‰6£8×Ø«ó:¢”ríA¸Š$ÁiP•kmT¡Qì68Œ)6cfGLW´JVáJß@ãdIØ,§å3Å.}n½g’$|O¦|Ñ¥ÆÆn='äòã‡h–Ø·â…+}†.}GÔcE¨ü ¯ÝÊNWvë¯êŠ¢HŠŒûurùAß?Ybßp
}Q%F±C¯ªG×¬ºX
ûÞ2ái,Ôÿ:¥ˆÆLs½.¥D|Q„7@^™S$ÁBø9r»aíøFàïg.=Éz¯ÖþAOH‘„VM|eÀ÷DÀ×Qø©Cñí~*—¯
¿xñ’ðð”¯MÀ¯þùØ¨¾ÃPbÌìŠòEsy¿ºt©$t°>SUR†ê›Æoý ÿÉ»#ª„ŠÆFt×5¦OÏ=!§útt®¤äç3eûúFÐoÒ–øÌô¡øv
¿ŽÂgÅo¢ðÀß¡‘ËàK2%á _ÔcVÿ$ ËP>w®~m„zfúzfÖðt;è½#ÐiÜZ’™ÎÊúþìp½$>çVäHÂòaâÝB|
½ÐgX"Wu¾ø§)ô.ýµFQ1Œ~Œø§qýºvh¾h]a•„lBÌµ‹à×[¹œ¯š¯2ÐKr#Ó™ý'Û“7´t¿*o¨¾Óx]È—„²>®1fú¢¹ýaöô ;5|4¾§£ÿ¦Ð¸ã0tV~Œç¥’p£&Þà{gðúšGÆ9êÖÛ_ñÍ”„»åöÕe¸Þ˜¹Vßí‹‘Ëú~Ð‹5ýHð™’ “û‰¹ÆD_TŒ½(ÓiÞ!]&	ßŽcæSˆËúÐûA¿uú!Ð+¾)	O#q˜w$H¼}Dà§ùÌÆèÅ —Ø$ak»@ô:Š–$<Ÿõÿ ÷‚¾0Ø/Eî—+Bý?ñí ŸhçýËHý¹ŠÕ?ø*
%áy¹õÌ^ðöÁêHæIhÐèiðë#à‹O˜+	ô¥Va>*µà{j˜tÚA_åúýuñtZÜ¬ß¼]¿ÌX²6ªÛÐÝ£tß¬4ß«˜'	?¦ÿ~Ÿòu¹$”ë}\Hº¸V?×XÂìæwÅ’ð;Ö^;Ô´|Ñ *öô’+$aÑË:¢¨s1.Ú?Ð3çKÂŸäò0N^«ïŽZ`,éˆfå'úIè’õ.S«wí ÷Ž@˜Ò/¾?ÛúþèïSù®”„Yþh|Rl¬PÙAæ¥ÒU’ðÈ0tz5è0èëxx5É ’¯ôžRu»+1&’¶EqûºX6<½ô VõçªþüaÐ×/ŒÌOz²ôÌ«%áj½¬ÿÝ¤ÿÚŒ)4VŽî0t1ý<Dù¼Fb<êq‘óêÞk¸½Vá“€o¼Vb5øàW]ËõU…§yüaà£4væõÂ·xýÎ£Ú¹œ$ÒÐéžm«Èó2¦ÿÀï~›‘ÒW{1jüOéÒy6íøx:×5_“ŸO)=àgÛËü°ö2ÑˆtA/Ê{6Ù©9ŠË]º^uªù¤Š^zE¹$ôë"ó{AÏ¬”„7uÊx–¾V
Ý3Ñï=¡
ú:ÿ&Ê_µ$\¤Ï	ÒÙø×Hò~~¨ß:ü*ào‹`'ˆÖG–å£õ’àSõrûîˆ¾Æ(­Õûbº£dý½×%	G S=Vƒ~øÅþ}Xlì]e ¹tw”ºÿC¸’zngøøMRåÞ†Íl„dM=ï ¾øq•Üú©_ u›7/¯<Ÿ¤Yd!ël¬O úÅœÉó_hà~7×GUþRÏl”„ûx|¨Öºô¬Þ(½bÐû¹	ï‡ÂS[‹	^E¨ÿjßúOžÖ—›$áÕÑð5ªæ¿à“<’Ðx’óìÔ	°S^Þî/­/DZw™Èe¶êý@@µìÀôòÐ
|§lïÐ–¯¤F}¥Q
ullý€Ö?.1Âºy€ÿçío…Ò¥¿qÞ;V®ðñŠ-—ý¼Ûzô²Ÿ›¡?FösqQ²_™c^ÊËýJžÏÑäGÑ÷Þ;äÛ Šlô¡rÓŸÒ¿+k*c¥0|-÷+¶Y±}qšônFçáÜ¯Èå0÷#	_ÆßÀ%a~ñû%Ãíäõó&‡px„Ã˜q2<—Ã‹9´r8‡Ã«9¬áp9‡þ€ÃG8|ŠÃ¾Éáá0f<OŸÃ‹9´r8‡Ã«9¬áp9‡þ€ÃGÆNþ	yýåbAøîáÞ€{	n7Üv¸ŸÃýî~¸ÁÝçƒ[w3œÎç„»®ôâ¡éÝÓ¡î†óÁ­€»®n6\ÜT¸ÉpÂX£þ
÷¸áá~÷ÜKp›á~
w7œîV¸Õp·ÀÝç‚+_£¦ëÙÈ/ÜÍpõp.¸epKàÁ-€+‚+„›——7îkpãáÆÀ}q– ¼{ÍcgÏž.¦Ì¿pª˜“nIÏ³23s3-–ibJéÂÒB±ØÕÐÜ:õ®±ÉÙä¬sVzœSÿ½™ÎÁ†…K÷Ôz¼MÞÊ¥Bº«ÁëljÒÜ^gºmVQš·r÷•6—
éµ•žZ!½zEƒgE½½MBú²†æôåÎ&ËÝæ)™«¤€üWc—’qá_¯³ÿÖÀš›ÎYéÎÚòš¦ÊzgymuSÈ'¤WyÝM$(ƒoW5±Ä+ë]UHÐíeÿÈqËñ,õ X•»¾ÞÙàý»íôXÞG(ýSïóô¡ñP¤~Vù;›÷U
ç/ãQÞ ñ_¤á÷r~¯>¼Ž?•÷ÝzMÞÃû8^éßc5ñdqè5ý}#ïÀ+8ŒáeÕô»Óy_¯×ŒúcÂÇÃÉoï«~¥>6&¼¿Wò¯×À¼ïWüJÿïˆ—¿6ÿÊßõ\zÍx£66rúÚò×jø•ñËÆØðñN4“iù›x¾b4ãµ{ÇŽ®þë5ü9ÿÆ±Aiø8lÕòóþy#0V×júeMú+5üÊørârÙî	òß®iç—8¿YÃ mO]þê›$eÿÃ†‘Ó¿OÃû>ÞüŽì?4!<¼VæüÊx÷hO	‡‘å¥å\ÃËÇ=±ßÿÏ4ü‰œ?ñû‘Ãký›yÝEiÆ©"çïÑ	aóXôjÒßÈÇûï9ÿ
|^Ã¯Ìz~ÝÈü¯hø÷óq÷þ;e‰ndýyÇ¥ðKwËü™wËþG£GæÿM›¦æÑlÿÔyWÿYï‘á{'èþ]þšœgCõéM#¹Ó¦1ˆ?´X³¬™‚%+;3;Û’›“+dZ2ss³1óË@³Ç[Ù$ŠB“Û=â¨çDô¯èßw
‹çèu!í×—±¶°‘”YåFÕB„$ä	qc8ŸÙ¯hU8IÂàQµcUëÕx04>”ÂÆ‡Išö©S„°òñ7ŒÇÂ{Ö`_ªô;JÂà5<«ôº0>=çKá|)<¼;Þ£:4¨Æ«¡ñ«fïDMÿ©ð)ãÔÐ¸U
·$jøJþè­fò4qyš¤0X¦Œ£5|W‚/æ$ôE)_Fk©5'£®:­Ž¦Di­yÖ4kNºÇžÅâNà2ÇT*LŽ^izÄ¦HV¯Sô‚xM|\§±ÑzMþ•>v7žçq,_›3ž <4Î9Kåÿ/^‹5øÉþXƒ¿›w25øw9œ¯Á·BãK5>…ÃtþA§ið_S3Õxeæ¿4ø:çjðùŠÐàsU2#È­JþÕze¤ðBù¼ååW9—¹<˜-Ï®«ôxœ¡¼|Y=f¼d½ååTªŠ4È*46aV]:M²ÝuålŽÝPYk]çt6ÒÉóªÆB£|lÞã®ºÁé\Noye5&Á§—pîF¯@½›×M§³½®z§»¦ºr¢¥”x²õ•®:™]çlªêÜ§0·¸hÖìò¬ô¬ôi²æFþOÖEþ3«ÆcÍ_wÅåqîú¬’ žšTâx›ãÇhÆûUxõg@…Wk«ðê1øQ>Ì^žÂ«Û}¬
?F=/QáÕcÜD^ÝFE^}~*E…«¨ðãTø<^Ý†ÙÕ”Iô„=¶2ðòþì[`Úçc„Àäcø—î½&“ŸM²÷ÓÂþä?“ŸD=ØÏüÈO"ìeþ÷ÈO¢ÜÈü¿%?‰tp=óï%?‰r°‡ù_$?‰ppóo#?ew°‘ùŸ%?‰l°‚ùF~Õ`	ó?F~Ñ Äü’ŸDÃ¾ ˜|ùI$ƒ¢j_Âòa‘ÿµëþ÷¾÷—”íê=<žÞ¾îŒv5bž<ÀÌýÓþ–^ôkñtÿ¾×«ô3Qõ¨ÿV\‹tº&äM8hOo¬Ã=„¹Ì©cé¯=ä³3:>Ýnõ•¨…_Á[±ôö	"8÷iVEHkÜVd‘ûn[à»UI†îË}º°Ô±úÐ½Ð~_`Œw’£ÓºuÀÈ.%¯§.Wï ’¥7ì‹++3é¹<öÙ²ï§¯J´™S‚(ÿóÍÄêß^D×ú‚x›ÿ]z£-“B'\Åøèú±¿Ìœw½Rª±ØÿñÀtÙÜ¿ÍñÚÐüÛú\¨ÛöÚçùð˜æÓ=Â½„_YZÜ½É¼
Ù£ÛÄïØLOÙ—,5²ÅþÏèvñmí,#êv¶›¿KÍÈÕtuþ¤s›=Ir°»­Õæ„ægì^×®v³ÄíAq÷s#]jÎÁq5œ®,ïÔØú^x]o3çQ°Ìßn¶ãgÝ¿Å<‹Gq*ŠÅÀ½àEÐ2&gÓæ2³±¦ïEã.óUì~?“ÊÞÍ¸×|¸òÏöîœKQlÛVi–Ipto1W°›þ3Ø“'vs2“êè.6KŽî»XféÁ Jq SO·[=þßYz·f``2ð»Â~‹Ãqé]¬¼$<zÌ¯–áÚÍ­¤!Ûu×^9Åqz»ÿs»ÿ(=ƒù…ÍôäÇ½Žü¾&£=ÿS;5­Áüãß5õURÑ;‘ÄÁ?lM¡äÎÃïÁ¿P¿7ê;®£'ŽÂÊY–½ô½¿1¾ôß.½nhßùØ÷‘®ÙhzjÌmdÚŒtÛüý«ß£Ùw4A×%ùà³~Óæ^ºÓøjø}AÄaó÷+IØ,»U	ô“@ŸÿEb£x·™6÷Ó`)ì¦É„®d‡Þ«ñNwƒéŽ°•Ì}]‰=¡û›ü¡Ö©Z&ZŽØž&³ö,…ôïH–ÉÏ·N”¿Ã!Ê×òÙçÅjzüïú|óoôÝYv5”Þ³ds)ÉµŸ˜ä“Ÿó$'I’¥÷*èô	aºÿºÐæÿbë:ôÌf8ü¯Ðó¢ªþR»;{¢gWô¦þæÒ®èŸ†~>£<y´ÿ˜‚xwFÞ	Ý7'ù¶ÇÖô½}ÿÆ
`lÐ]ƒíéUHÉrÄÐûûüŸû£kAê,4ø£«(Õ¾Æý-xL›ñó¥øIÕ{.}g1z6|+'˜6ë|PÇ}zÝNßçTÇtÞü6mÞé¨zui÷l{†‘·=G)úûI—¶±G>ÛkÚüâšÝ¦Â>Ëî›_úúÎè7£„BÛN)Vªý9·ˆ^ÏøíÀÿŠÆ“ÿ¶é®^zOH¶“¦§z}ºí¦§tP§s)»çêv\Ò÷ï³Ôæ>ëŠ£ì>ësúÓqŸU}_µB¯¹¯z4at÷U§ü5x_U¹ŸÚ¼ŸJ#Â¥Í5¢§©ÊÕHÿ6º›¼bµÇþe¾š&çÍÎ†ª¡ûƒò8Òã¼Q¾GwÚÊøØ'ÉÑ™ÄÐLöÓXf†^J¹ÖÙŽ¦ ÖÂòÐë¯tw|à€ô”lj½Qß¿îÊÄÀÉÝCÉá|4.èÐx^c®±µA9±¡ÜCÆJÂUî¡TŒ•‚sÉ°{(ÀßÀ&¤ÆÌà9lçãøRoæ÷PÖ?G§œ;›gLì¢Ë†¹Æ:B(è|Hæ‘Ï‡¤|1òùÔ91áç@vG>Rk?2 ?²Y8És g…ŸÉÓ|¤5:üHE´qÿe¸s RôÈç@Äè/çHp^1!òùßñzùˆÃãùºÖ¦sx‡ó9¼žÃWr¸–ÃsÞ%QÉÏÎ	ügWþŒÛ•ÿ×ß—/;É}yÅþ*ûÉ«t'·/¯ØóZC¸ýí¾¼b/Åèpùv_^±ÇRô©íË+ö¿":¼œ£Ý—Wú“Öèðþg´ûòJÿ¹1æÔöå_àü/pÄ~á$÷å¹ß¨œsrûòJ¿Ãxjûòû9ÿ~ÎÍ	ö•µûò¡uþpýí¾|çïáüuº“Û—í£œÚ¾|çO%¿v_^äüâ„SÛ—íçœÚ¾¼²¾ûôY#§?ì¾<çïåü«„“Û——ø:©tväö{‚}ù
ÎÍÙŠýÐÒ¾¼ÂïÕÚ¾üž‰ÿÙ—ÿçíÿcàôåìÿ[rs­ÌoÉ±*ç 2­–,Á’e±Z­¹Y™9Fv™™_òþÿ‰þj+ëª+õî†­úŸtQÆRWC†§6¾ÄVæ(Èhö4eÔ¹«*ë2<ÀOWùƒÞý½ø'>¾¦¹¡Ê‹Ñ²È^K™s|œ³ªÖ-&›3G·Þ"^GÏa-I_«Ë;#þ–øxWxhž$¦9o§‰KÄ¢·ÖÙG1nv$3Æh99ÞY§mÚlE(3>Žå1¾Æ‘AEU¥W4+¡Ä6±²åqÊÍ5î¦We†kfÁü93\—^:õfyo2ÅìšzË-S*âÁPPálmlñk™·V4#ºŠøI©ùfü;}šu:H(Iz†ÜÚXÑlÍY¢9[4çˆæiññÿœú÷8—-÷ºÝu§ÓŒ|þ''sš5—ÎÿX²rr³raÐþÿsþçËmÿK+ÿ1`½—Öº<¾ª¹‰˜È˜-ñ4—õÖ7Rû*H6«ˆååDKŽgï„“ã«›ëëaƒ›V !5×y#G‡@år¨r9Xº·¶!dìq ›¨ô½_hVe—Þ04«3'^z„ŒˆY—]l‰cv$³`JÑîÊÐî†XJù!ÅdQe9—RâM•UNñºÆJomFµ³²ŽÞÌ[N/ 2[©„¤tÕþz~e^¼®%'£%/£Åb]"^Wëöx3œ^üÌÎÊ°æ,ÂD8	žeNoãËØÃrzìtÆ »È9T‡ç†ðäúµü®r–"‚ÑHï”’âìŒiºÈÿ<îæ&T‚«QM§¦‡Ñ	Á
ÆPí„·iª:žéC…ÅÌ&Ø)£°±ÑÙÄ}…¤÷3•D©TB³ŠHE5•PKz‹pqrz†Ò.Nò"Òö²æh}òÊÖ’£dÒYYU+6¹[Äqé
¯Ó
’7$Hž6ˆÅ:]ÄbÕ„¡<È §åNÔ9aX ÑÝTMFºEäQ…	òM¬qáäT±Åå­¥7 åi•0È’¥ä:;K\êòŠMîeM•õ©¢§¹QV9w“RÏˆ5g>Î4¤ªè±ÔQ×T¨…ÿKÔRËë••þŒ®§YhedË‡«§“·ûD›®jèƒºkÄ°Àa->£Ö]ïÌp»ªnD¿¾,#<sÞÙTŒF‘NÊì‡BQn›=NÆ‡Z’ßŒ‡•ŒùDò:=!uA
!#ÇÒW§p+Ø¢¼Â+§3´3‘Ë:ÜB5ßp„&qá³ÜQ0›sMGâT=|4ý¡YK<ÏÑElœŸ¨¢–ËÊž3ÑÜ^zÍ×ãMOæ‘Íà±)“øÕóÑl±ÇÓ0	ÅÇÕ¯hqU{krè'¢rU6°¶›LÏ
×Y_€¶•¬’&ºž,u®«*QÿHßEž¸–œ¶œ©ôCu\ÜŒŒ–×–FËSÑ,Ö6‹5Œj±†È—0Š¦#HœÓSY¥–2ÏiöÐœfóœR1ÛjyŠš²+ÉÂ µ5h‚ —ü÷g,ghÆrxÆ²³Ú²yša5 $iÍi³jèÖF—àòcª–L\Ðæ1K'33Sl™Æ|ë5"—,X4*Õ©BnÇßÃ´‰ˆaCíC\ánë1›š0â£èÅÉ°MÚ³Uüì0¤sÊ°C~±M\Öäl“U¯“'‹i6gµ˜Ö NÉjœÂWÒæ¤‰SäåÑl¹eÊÔ¼Ìi7ÁØÞ¬Mø–ä¡%fÏ”³´J™Å´‘!f'Ï5Q‰3lz3ŒÃÂ¨ÄwEåŠ¥Î“"¥=Dç.¾ÅTZLr«X ðk‰*OnˆÍÖ*¦y[-bZKŽ˜¶\“7eéÆl)HÎlM6[f@šì yCs}ŠÙ2u†ÙriÁ”d³V¢ÉSfðÅ0NÎKÏk'{äÿ7$§"šTó´TsNª9;Õœ5õª¸å.g‹˜¦ØþÑ”‰,ÁU¤,V”jšºHâ0eÊe=1õ¢p²ÁÃÁâ´†Ù–ð¢ç¾¢³ „
?Dù©æ¼TsnªÙ:\Ç‘Ë]õ——í°AÕËt\¼lpG_—Cõó+V•!}fµ™Ë“†Wå(2=bUb.uúÚ%"ÿ2jsÿœ ½¦š-@Y€³ iÖ´…ðà±d†¸áµàt–7¨'SÚÈ][:mé5æüd•çcÎ@Ý‰ÜÖ¸-\¢Ù\¦q‰æj¥ùQ@aÃqÉiÓÂ3f\";Ã¦'“¯ÿÌÿé³@Z¹:ÙI`èSV§<Ô$;Ü¥tR3@õ†×pT9Ãæê¬ÞØj„y&Íþ¾´is¿ÓPðÓ?ó;ÝµõÕ›÷ýªñïŸõ}éÕøU˜óÞš<Óf|§Sþ½æ{gŽÞ|	³½3EmNß\/´-,ï’B%”Ús6ùé¤Ó5¶ÍêuG8šõí]mOGþl~Åæä¨Xò	Û	mjr 5U¤6­ò¨d°!Ü³…M@ÂÎoïÌ¾ïúlŸM›Òê		ßÎ¾ÍÎÎìîÍ>Wß—ïÍÇa³D~qY×ùX¾ºÍZOf…lL¹¥³>ïRÛ¯“¿Õn òv Ú~Àn±d§ÝÂÜ½Ççg¬«•¨©
ýìiœÕ›Kªï‹ñíÔ‹¸»Wr8‚¶íèÍ¢<"™Mâ±Ð‚†á›½ «þ†®â+ºŠ×µÉˆÄy·="r÷Å¸É$·î;Î•˜!SÎDN¡À”ØôsÝëH?Ô¥½)7¼ùåî‡wYFvÍ„j'ÙÑm(‹PÈÊÛ8Þ)	—­áŠò¦ÈŸw²¬u”$ÝD$ƒ¹a¡–pt9‡šzöTX‹|QºKrõ¸õIkž: eQìj¶l/÷ÒãMÞ
žg‡ï>çâß{ÅµqŸÃÑh,¿äÍÑÑ5ŽËù³ÙõÝ	¯ÏxÂ[š^ýµ@µQPƒùA1(+SGWÊ.FÁ^ATX²†;µ}}°%„>¼ÈÞÕ7­y—yf“¼<(i˜ZiSMV·þ 3Í=K¼Ç_`ïŠ_lBßÿÜÝ“–yÚË‡"m[5ôì³”ÃÅ¨àûÙT¶´á_”]ãÚÈdëŠÉR+ýý(3ËÐº¬i„kQÃÄ—}ôúâxt3à€rñËèÊEïeUÉÅû¹¦>¢”DžÃ_P›ØÝåcêõbñ$ªšô{)¥DB=i¥?ž>´›/Oç‚£uÏOš½ô²ù&}gRº%;ô\•­œeÒ›²X;r™i»‰+•¤šI4ˆ;¤ù£‹?{K;cõCõbUÎu¯é?ú©è+aþïÚ}ûo½Ïªb£-•"–Ö	í·sR¾‚Š°B{ABÓÑŒ|ÝÒrùlICxu:%]^ï&•¦oHaýÍý~³LU=ÇZSh¤½ÐS-o®ùîÏÜÖtž+m‹ÔÊk.§%nÓ„gš\~±÷q
™[£…QG¥tø?—ÒiÐä´¿¹ØÆ!NÞÅ†O_gÄ¬µ‡¬OÑçåU2—¶À¬ATÛ•#´,á––6Æ÷iþÊ{Xz1,çKaâ«ØûãtoÏÚŽ\WîV@ºhÕ€¨bkäOžOÎÈ‰@x’ãœãÚ{…’O&#ÝTie]Vq&ŸzT„\L×š4IÝä§Åª×õÌÛä­iD½-ì=ùZtZzuê)ÿ­OÃº]W#Ëfd%ë
%%+âÌn¯ÕVûD¤—"=&¯ç‹k±;‰ÿ‹V$#¨Ï µÇ ß™?Ùgs”vV¡ˆÎØ×“S©ÎÑQ·ÅêS¢=ª…NÕdb>!L"e·DUØJÉ—zÌ6ô•²˜ú]	Tï³,b›â–+'†C¿TcivQ›m=_£ª®"V„3Ò„ ç®$ÃyZœ²;—8]®©Œ|kl§gck]bÛÚ>2«?oÔJ£=wó&ò,gfYéd™Ó<‹f­’-WÝÌš›®ý›–|Sþ÷Ûr7#êfÄD:œ†áD¬%yq;80ZïO¥5n ævZäB8³^g™²(%nÃŒ’Jv>#lÇ3Úpì†f¯!·üØAÂ¶±ýv
nþK[÷mo~oé—ÖÎöÔ3-ëZ™K2ýô¦zä¬ %ßÆÞ(Ì¾B!õ!ô[[;®Øsmãº*û®ÕÎë1Þk+÷µÜ™·l•³³aZ—x1£ s¿ çÃœ‹”žËž	ø'×îNæìŽÏLýí~‡å‚o(k|•éÅRNÁ‡ÿ!óËUÌ‚_„üûkøÁÏr5]–EŸ<²Hø`ÒÞrœÝõ×v]‘Wé¹f¬ÔqÇ[©ß{¥n{Qc¦×lÆgý[ê°»æ®Rìòev>]%{”´R–KKBÎ…ðv£k”º'8»7V\°sGÈÑÅ4[üÂ!³)_]i›ÝÄ*ÜÁSì.½ýwóU•L¢þªüæžiŸŠ—ü¤‰<2øþÇ×'€ÿAŒÿÑi¿üà ÿøÀÿ þð?€ÿüà ÿøÀÿÀÍ/à ÿøÀÿ þð?€ÿüà ÿøÀÿ þvÀÿ þð?€ÿüà ÿøÀÿ þð?€ÿüà ÿøÀÿ þð?€ÿüà ÿøÀÿ þð?€ÿüà<
ÿ£üà ÿøÀÿ þð?€ÿñ_Àÿ @ @ @ ý¿è/NÜ‹ h 