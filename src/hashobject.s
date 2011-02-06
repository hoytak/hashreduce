	.file	"hashobject.c"
	.text
	.p2align 4,,15
.globl md5_process
	.type	md5_process, @function
md5_process:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$76, %esp
	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	movl	8(%ebp), %esi
	movl	(%edx), %edx
	movl	4(%esi), %edi
	movl	8(%esi), %ebx
	movl	%edx, -88(%ebp)
	movl	(%eax), %edx
	movl	12(%esi), %ecx
	movl	2(%eax), %esi
	movl	%edx, -84(%ebp)
	movl	4(%eax), %edx
	movl	%esi, -80(%ebp)
	movl	6(%eax), %esi
	movl	%edx, -76(%ebp)
	movl	8(%eax), %edx
	movl	%esi, -72(%ebp)
	movl	10(%eax), %esi
	movl	%edx, -68(%ebp)
	movl	12(%eax), %edx
	movl	%esi, -64(%ebp)
	movl	14(%eax), %esi
	movl	%edx, -60(%ebp)
	movl	16(%eax), %edx
	movl	%esi, -56(%ebp)
	movl	18(%eax), %esi
	movl	%edx, -52(%ebp)
	movl	20(%eax), %edx
	movl	%esi, -48(%ebp)
	movl	%edx, -44(%ebp)
	movl	22(%eax), %esi
	movl	24(%eax), %edx
	movl	%esi, -40(%ebp)
	movl	26(%eax), %esi
	movl	%edx, -36(%ebp)
	movl	28(%eax), %edx
	movl	30(%eax), %eax
	movl	%esi, -32(%ebp)
	movl	-88(%ebp), %esi
	movl	%edx, -28(%ebp)
	movl	%ebx, %edx
	movl	%eax, -24(%ebp)
	movl	%edi, %eax
	andl	%edi, %edx
	notl	%eax
	andl	%ecx, %eax
	orl	%edx, %eax
	leal	-680876936(%esi,%eax), %edx
	movl	-80(%ebp), %eax
	addl	-84(%ebp), %edx
	rorl	$25, %edx
	addl	%edi, %edx
	leal	-389564586(%eax,%ecx), %esi
	movl	%edx, %eax
	movl	%edx, %ecx
	notl	%eax
	andl	%edi, %ecx
	andl	%ebx, %eax
	orl	%ecx, %eax
	addl	%eax, %esi
	movl	-76(%ebp), %eax
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %ecx
	leal	606105819(%eax,%ebx), %ebx
	movl	%esi, %eax
	notl	%eax
	andl	%edx, %ecx
	andl	%edi, %eax
	orl	%ecx, %eax
	addl	%eax, %ebx
	movl	-72(%ebp), %eax
	rorl	$15, %ebx
	addl	%esi, %ebx
	leal	-1044525330(%eax,%edi), %ecx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-68(%ebp), %edi
	addl	%eax, %ecx
	rorl	$10, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	leal	-176418897(%edx,%edi), %edx
	notl	%eax
	movl	%ecx, %edi
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-64(%ebp), %eax
	rorl	$25, %edx
	addl	%ecx, %edx
	movl	%edx, %edi
	leal	1200080426(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	movl	-60(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	leal	-1473231341(%ebx,%edi), %ebx
	notl	%eax
	movl	%esi, %edi
	andl	%ecx, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-56(%ebp), %eax
	rorl	$15, %ebx
	addl	%esi, %ebx
	movl	%ebx, %edi
	leal	-45705983(%ecx,%eax), %ecx
	movl	%ebx, %eax
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-52(%ebp), %edi
	addl	%eax, %ecx
	rorl	$10, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	leal	1770035416(%edx,%edi), %edx
	notl	%eax
	movl	%ecx, %edi
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-48(%ebp), %eax
	rorl	$25, %edx
	addl	%ecx, %edx
	movl	%edx, %edi
	leal	-1958414417(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	movl	-44(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	leal	-42063(%ebx,%edi), %ebx
	notl	%eax
	movl	%esi, %edi
	andl	%ecx, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-40(%ebp), %eax
	rorl	$15, %ebx
	addl	%esi, %ebx
	movl	%ebx, %edi
	leal	-1990404162(%ecx,%eax), %ecx
	movl	%ebx, %eax
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-36(%ebp), %edi
	addl	%eax, %ecx
	rorl	$10, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	leal	1804603682(%edx,%edi), %edx
	notl	%eax
	movl	%ecx, %edi
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-32(%ebp), %eax
	rorl	$25, %edx
	addl	%ecx, %edx
	movl	%edx, %edi
	leal	-40341101(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	addl	%eax, %esi
	movl	-28(%ebp), %eax
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %edi
	notl	%edi
	movl	%edi, -20(%ebp)
	leal	-1502002290(%ebx,%eax), %ebx
	movl	%edi, %eax
	movl	%esi, %edi
	andl	%ecx, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-24(%ebp), %eax
	rorl	$15, %ebx
	addl	%esi, %ebx
	movl	%ebx, %edi
	notl	%edi
	movl	%edi, -16(%ebp)
	leal	1236535329(%ecx,%eax), %ecx
	movl	%edi, %eax
	movl	%ebx, %edi
	andl	%edx, %eax
	andl	%esi, %edi
	orl	%edi, %eax
	movl	-80(%ebp), %edi
	addl	%eax, %ecx
	rorl	$10, %ecx
	addl	%ebx, %ecx
	andl	%ebx, -20(%ebp)
	movl	%ecx, %eax
	andl	%esi, %eax
	orl	-20(%ebp), %eax
	leal	-165796510(%edx,%edi), %edx
	addl	%eax, %edx
	movl	-60(%ebp), %eax
	rorl	$27, %edx
	addl	%ecx, %edx
	andl	%ecx, -16(%ebp)
	leal	-1069501632(%esi,%eax), %esi
	movl	%edx, %eax
	movl	-40(%ebp), %edi
	andl	%ebx, %eax
	orl	-16(%ebp), %eax
	leal	643717713(%ebx,%edi), %ebx
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%edx, %esi
	andl	%edx, %eax
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-84(%ebp), %eax
	addl	%edi, %ebx
	rorl	$18, %ebx
	addl	%esi, %ebx
	leal	-373897302(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ecx
	movl	-64(%ebp), %edi
	notl	%eax
	rorl	$12, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	leal	-701558691(%edx,%edi), %edx
	movl	%ecx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-44(%ebp), %eax
	addl	%edi, %edx
	rorl	$27, %edx
	addl	%ecx, %edx
	leal	38016083(%esi,%eax), %esi
	movl	%ebx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %esi
	movl	-24(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	addl	%edx, %esi
	andl	%edx, %eax
	leal	-660478335(%ebx,%edi), %ebx
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-68(%ebp), %eax
	addl	%edi, %ebx
	rorl	$18, %ebx
	addl	%esi, %ebx
	leal	-405537848(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ecx
	movl	-48(%ebp), %edi
	notl	%eax
	rorl	$12, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	leal	568446438(%edx,%edi), %edx
	movl	%ecx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-28(%ebp), %eax
	addl	%edi, %edx
	rorl	$27, %edx
	addl	%ecx, %edx
	leal	-1019803690(%esi,%eax), %esi
	movl	%ebx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %esi
	movl	-72(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	leal	-187363961(%ebx,%edi), %ebx
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-52(%ebp), %eax
	addl	%edi, %ebx
	rorl	$18, %ebx
	addl	%esi, %ebx
	leal	1163531501(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ecx
	movl	-32(%ebp), %edi
	notl	%eax
	rorl	$12, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	leal	-1444681467(%edx,%edi), %edx
	movl	%ecx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-76(%ebp), %eax
	addl	%edi, %edx
	rorl	$27, %edx
	addl	%ecx, %edx
	leal	-51403784(%esi,%eax), %esi
	movl	%ebx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %esi
	movl	-56(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	leal	1735328473(%ebx,%edi), %ebx
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-36(%ebp), %eax
	addl	%edi, %ebx
	rorl	$18, %ebx
	addl	%esi, %ebx
	leal	-1926607734(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %ecx
	movl	-64(%ebp), %edi
	xorl	%esi, %eax
	rorl	$12, %ecx
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-378558(%edx,%edi), %edx
	movl	-40(%ebp), %edi
	addl	%eax, %edx
	movl	-52(%ebp), %eax
	rorl	$28, %edx
	addl	%ecx, %edx
	leal	-2022574463(%esi,%eax), %esi
	movl	%ecx, %eax
	xorl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	1839030562(%ebx,%edi), %ebx
	movl	-80(%ebp), %edi
	addl	%eax, %ebx
	movl	-28(%ebp), %eax
	rorl	$16, %ebx
	addl	%esi, %ebx
	leal	-35309556(%ecx,%eax), %ecx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$9, %ecx
	xorl	%esi, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-1530992060(%edx,%edi), %edx
	movl	-56(%ebp), %edi
	addl	%eax, %edx
	movl	-68(%ebp), %eax
	rorl	$28, %edx
	addl	%ecx, %edx
	leal	1272893353(%esi,%eax), %esi
	movl	%ecx, %eax
	xorl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	-155497632(%ebx,%edi), %ebx
	movl	-32(%ebp), %edi
	addl	%eax, %ebx
	movl	-44(%ebp), %eax
	rorl	$16, %ebx
	addl	%esi, %ebx
	leal	-1094730640(%ecx,%eax), %ecx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$9, %ecx
	xorl	%esi, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	681279174(%edx,%edi), %edx
	movl	-72(%ebp), %edi
	addl	%eax, %edx
	movl	-84(%ebp), %eax
	rorl	$28, %edx
	addl	%ecx, %edx
	leal	-358537222(%esi,%eax), %esi
	movl	%ecx, %eax
	xorl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	-722521979(%ebx,%edi), %ebx
	movl	-48(%ebp), %edi
	addl	%eax, %ebx
	movl	-60(%ebp), %eax
	rorl	$16, %ebx
	addl	%esi, %ebx
	leal	76029189(%ecx,%eax), %ecx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$9, %ecx
	xorl	%esi, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-640364487(%edx,%edi), %edx
	movl	-24(%ebp), %edi
	addl	%eax, %edx
	movl	-36(%ebp), %eax
	rorl	$28, %edx
	addl	%ecx, %edx
	leal	-421815835(%esi,%eax), %esi
	movl	%ecx, %eax
	xorl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	530742520(%ebx,%edi), %ebx
	movl	-84(%ebp), %edi
	addl	%eax, %ebx
	movl	-76(%ebp), %eax
	rorl	$16, %ebx
	addl	%esi, %ebx
	leal	-995338651(%ecx,%eax), %ecx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$9, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-198630844(%edx,%edi), %edx
	movl	-28(%ebp), %edi
	addl	%eax, %edx
	movl	-56(%ebp), %eax
	rorl	$26, %edx
	addl	%ecx, %edx
	leal	1126891415(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1416354905(%ebx,%edi), %ebx
	movl	-36(%ebp), %edi
	addl	%eax, %ebx
	movl	-64(%ebp), %eax
	rorl	$17, %ebx
	addl	%esi, %ebx
	leal	-57434055(%ecx,%eax), %ecx
	movl	%edx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	1700485571(%edx,%edi), %edx
	movl	-44(%ebp), %edi
	addl	%eax, %edx
	movl	-72(%ebp), %eax
	rorl	$26, %edx
	addl	%ecx, %edx
	leal	-1894986606(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1051523(%ebx,%edi), %ebx
	movl	-52(%ebp), %edi
	addl	%eax, %ebx
	movl	-80(%ebp), %eax
	rorl	$17, %ebx
	addl	%esi, %ebx
	leal	-2054922799(%ecx,%eax), %ecx
	movl	%edx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	1873313359(%edx,%edi), %edx
	movl	-60(%ebp), %edi
	addl	%eax, %edx
	movl	-24(%ebp), %eax
	rorl	$26, %edx
	addl	%ecx, %edx
	leal	-30611744(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1560198380(%ebx,%edi), %ebx
	movl	-68(%ebp), %edi
	addl	%eax, %ebx
	movl	-32(%ebp), %eax
	rorl	$17, %ebx
	addl	%esi, %ebx
	leal	1309151649(%ecx,%eax), %ecx
	movl	%edx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-145523070(%edx,%edi), %edx
	movl	-76(%ebp), %edi
	addl	%eax, %edx
	movl	-40(%ebp), %eax
	rorl	$26, %edx
	addl	%ecx, %edx
	leal	-1120210379(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	leal	718787259(%ebx,%edi), %ebx
	movl	-88(%ebp), %edi
	notl	%eax
	rorl	$22, %esi
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	leal	(%edx,%edi), %eax
	movl	8(%ebp), %edi
	notl	%edx
	rorl	$17, %ebx
	addl	%esi, %ebx
	orl	%ebx, %edx
	movl	%eax, (%edi)
	movl	%ebx, %eax
	xorl	%esi, %edx
	addl	4(%edi), %eax
	movl	-48(%ebp), %edi
	leal	-343485551(%ecx,%edi), %ecx
	addl	%edx, %ecx
	movl	8(%ebp), %edx
	rorl	$11, %ecx
	addl	%ecx, %eax
	addl	%ebx, 8(%edx)
	addl	%esi, 12(%edx)
	movl	%eax, 4(%edx)
	addl	$76, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	md5_process, .-md5_process
	.p2align 4,,15
.globl md5_finish
	.type	md5_finish, @function
md5_finish:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %edx
	movl	8(%ebp), %ecx
	movl	(%edx), %eax
	movl	%eax, 8(%ecx)
	movl	4(%edx), %eax
	movl	%eax, 12(%ecx)
	movl	8(%edx), %eax
	movl	%eax, 16(%ecx)
	movl	12(%edx), %eax
	movl	%eax, 20(%ecx)
	popl	%ebp
	ret
	.size	md5_finish, .-md5_finish
	.p2align 4,,15
.globl H_IsHashed
	.type	H_IsHashed, @function
H_IsHashed:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	movzbl	24(%eax), %eax
	shrb	%al
	andl	$1, %eax
	ret
	.size	H_IsHashed, .-H_IsHashed
	.p2align 4,,15
.globl H_Freeze
	.type	H_Freeze, @function
H_Freeze:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	orb	$1, 24(%eax)
	popl	%ebp
	ret
	.size	H_Freeze, .-H_Freeze
	.p2align 4,,15
.globl H_IsFrozen
	.type	H_IsFrozen, @function
H_IsFrozen:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	movzbl	24(%eax), %eax
	andl	$1, %eax
	ret
	.size	H_IsFrozen, .-H_IsFrozen
	.p2align 4,,15
.globl H_IncRef
	.type	H_IncRef, @function
H_IncRef:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx
	movl	4(%edx), %eax
	testl	%eax, %eax
	js	.L13
	addl	$1, %eax
	movl	%eax, 4(%edx)
.L13:
	popl	%ebp
	ret
	.size	H_IncRef, .-H_IncRef
	.p2align 4,,15
.globl H_RefCount
	.type	H_RefCount, @function
H_RefCount:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	movl	4(%eax), %eax
	ret
	.size	H_RefCount, .-H_RefCount
	.p2align 4,,15
.globl H_Equal
	.type	H_Equal, @function
H_Equal:
	pushl	%ebp
	movl	$1, %eax
	movl	%esp, %ebp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	cmpl	%ecx, %edx
	je	.L19
	movl	8(%edx), %eax
	cmpl	8(%ecx), %eax
	je	.L22
.L20:
	xorl	%eax, %eax
.L19:
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L22:
	movl	12(%edx), %eax
	cmpl	12(%ecx), %eax
	jne	.L20
	movl	16(%edx), %eax
	cmpl	16(%ecx), %eax
	jne	.L20
	movl	20(%edx), %eax
	cmpl	20(%ecx), %eax
	popl	%ebp
	sete	%al
	ret
	.size	H_Equal, .-H_Equal
	.p2align 4,,15
.globl H_ExtractHash
	.type	H_ExtractHash, @function
H_ExtractHash:
	pushl	%ebp
	xorl	%edx, %edx
	movl	%esp, %ebp
	movl	12(%ebp), %ecx
	pushl	%ebx
	movl	8(%ebp), %ebx
	testl	%ecx, %ecx
	je	.L30
.L28:
	movl	8(%ecx,%edx,4), %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, (%ebx,%edx,8)
	movl	8(%ecx,%edx,4), %eax
	shrl	$4, %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 1(%ebx,%edx,8)
	movl	8(%ecx,%edx,4), %eax
	shrl	$8, %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 2(%ebx,%edx,8)
	movl	8(%ecx,%edx,4), %eax
	shrl	$12, %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 3(%ebx,%edx,8)
	movzwl	10(%ecx,%edx,4), %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 4(%ebx,%edx,8)
	movl	8(%ecx,%edx,4), %eax
	shrl	$20, %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 5(%ebx,%edx,8)
	movzbl	11(%ecx,%edx,4), %eax
	andl	$15, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 6(%ebx,%edx,8)
	movl	8(%ecx,%edx,4), %eax
	shrl	$28, %eax
	movzbl	map.6834(%eax), %eax
	movb	%al, 7(%ebx,%edx,8)
	addl	$1, %edx
	cmpl	$4, %edx
	jne	.L28
.L27:
	popl	%ebx
	popl	%ebp
	ret
.L30:
	leal	4(%ebx), %eax
	movb	$78, (%ebx)
	movb	$85, 1(%ebx)
	movb	$76, 2(%ebx)
	movb	$76, 3(%ebx)
	movl	$538976288, 4(%ebx)
	movl	$538976288, 4(%eax)
	movl	$538976288, 8(%eax)
	movl	$538976288, 12(%eax)
	movl	$538976288, 16(%eax)
	movl	$538976288, 20(%eax)
	movl	$538976288, 24(%eax)
	jmp	.L27
	.size	H_ExtractHash, .-H_ExtractHash
	.p2align 4,,15
.globl H_debug_PrintHash
	.type	H_debug_PrintHash, @function
H_debug_PrintHash:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$68, %esp
	movl	%gs:20, %eax
	movl	%eax, -8(%ebp)
	xorl	%eax, %eax
	movl	8(%ebp), %eax
	leal	-41(%ebp), %ebx
	movl	%ebx, (%esp)
	movl	%eax, 4(%esp)
	call	H_ExtractHash
	movb	$0, -9(%ebp)
	movl	%ebx, (%esp)
	call	puts
	movl	-8(%ebp), %eax
	xorl	%gs:20, %eax
	jne	.L34
	addl	$68, %esp
	popl	%ebx
	popl	%ebp
	ret
.L34:
	.p2align 4,,6
	.p2align 3
	call	__stack_chk_fail
	.size	H_debug_PrintHash, .-H_debug_PrintHash
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"HashObject Double Delete."
.LC1:
	.string	"hashobject.h"
.LC2:
	.string	"\n%s:%d:\t%s.\n\n"
	.text
	.p2align 4,,15
.globl H_DecRef
	.type	H_DecRef, @function
H_DecRef:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %edx
	movl	4(%edx), %eax
	testl	%eax, %eax
	jle	.L36
	subl	$1, %eax
	testl	%eax, %eax
	movl	%eax, 4(%edx)
	jne	.L39
	movl	(%edx), %eax
	movl	4(%eax), %ecx
	testl	%ecx, %ecx
	je	.L38
	leave
	jmp	*%ecx
	.p2align 4,,7
	.p2align 3
.L39:
	leave
	.p2align 4,,3
	.p2align 3
	ret
	.p2align 4,,7
	.p2align 3
.L36:
	movl	$.LC0, 20(%esp)
	movl	$356, 16(%esp)
	movl	$.LC1, 12(%esp)
	movl	$.LC2, 8(%esp)
	movl	$1, 4(%esp)
	movl	stderr, %eax
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	stderr, %eax
	movl	%eax, 8(%ebp)
	leave
	jmp	fflush
	.p2align 4,,7
	.p2align 3
.L38:
	movl	%edx, 8(%ebp)
	leave
	jmp	free
	.size	H_DecRef, .-H_DecRef
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align 4
.LC3:
	.string	"FillHash called with Hash Function Null."
	.section	.rodata.str1.1
.LC4:
	.string	"\nWARNING %s"
.LC5:
	.string	"1"
	.section	.rodata.str1.4
	.align 4
.LC6:
	.string	"\nTriggered in %s, function %s, line %d, %s"
	.text
	.p2align 4,,15
.globl H_EnsureHashed
	.type	H_EnsureHashed, @function
H_EnsureHashed:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$36, %esp
	movl	8(%ebp), %ebx
	testb	$2, 24(%ebx)
	jne	.L43
	movl	(%ebx), %eax
	movl	8(%eax), %ecx
	testl	%ecx, %ecx
	je	.L44
	movl	%ebx, 8(%ebp)
	addl	$36, %esp
	popl	%ebx
	popl	%ebp
	jmp	*%ecx
	.p2align 4,,7
	.p2align 3
.L44:
	movl	stderr, %eax
	movl	$.LC3, 12(%esp)
	movl	$.LC4, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	stderr, %eax
	movl	$.LC5, 24(%esp)
	movl	$259, 20(%esp)
	movl	$__func__.3161, 16(%esp)
	movl	$.LC1, 12(%esp)
	movl	$.LC6, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	stderr, %eax
	movl	%eax, (%esp)
	call	fflush
	movl	$0, 8(%ebx)
	movl	$0, 12(%ebx)
	movl	$0, 16(%ebx)
	movl	$0, 20(%ebx)
	.p2align 4,,7
	.p2align 3
.L43:
	addl	$36, %esp
	popl	%ebx
	popl	%ebp
	ret
	.size	H_EnsureHashed, .-H_EnsureHashed
	.p2align 4,,15
.globl H_New
	.type	H_New, @function
H_New:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$32, (%esp)
	call	malloc
	movl	%eax, %edx
	xorl	%eax, %eax
.L47:
	movl	$0, (%edx,%eax)
	addl	$4, %eax
	cmpl	$32, %eax
	jb	.L47
	movl	4(%edx), %eax
	movl	$HashObject_TypeStruct, (%edx)
	testl	%eax, %eax
	js	.L49
	addl	$1, %eax
	movl	%eax, 4(%edx)
.L49:
	movl	%edx, %eax
	leave
	ret
	.size	H_New, .-H_New
	.p2align 4,,15
.globl Hf_OICombine
	.type	Hf_OICombine, @function
Hf_OICombine:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movl	%ebx, (%esp)
	movl	16(%ebp), %ebx
	movl	%esi, 4(%esp)
	movl	12(%ebp), %esi
	testl	%edx, %edx
	je	.L55
.L53:
	movl	8(%ebx), %eax
	addl	8(%esi), %eax
	movl	%eax, 8(%edx)
	movl	12(%ebx), %eax
	addl	12(%esi), %eax
	movl	%eax, 12(%edx)
	movl	16(%ebx), %eax
	addl	16(%esi), %eax
	movl	%eax, 16(%edx)
	movl	20(%ebx), %eax
	addl	20(%esi), %eax
	movl	%eax, 20(%edx)
	movl	%edx, %eax
	movl	(%esp), %ebx
	movl	4(%esp), %esi
	movl	%ebp, %esp
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L55:
	call	H_New
	movl	%eax, %edx
	jmp	.L53
	.size	Hf_OICombine, .-Hf_OICombine
	.p2align 4,,15
.globl md5_fill_dest_key
	.type	md5_fill_dest_key, @function
md5_fill_dest_key:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ebx
	testl	%edx, %edx
	je	.L60
.L58:
	movl	(%ebx), %eax
	movl	%eax, 8(%edx)
	movl	4(%ebx), %eax
	movl	%eax, 12(%edx)
	movl	8(%ebx), %eax
	movl	%eax, 16(%edx)
	movl	12(%ebx), %eax
	movl	%eax, 20(%edx)
	addl	$4, %esp
	movl	%edx, %eax
	popl	%ebx
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L60:
	call	H_New
	movl	%eax, %edx
	jmp	.L58
	.size	md5_fill_dest_key, .-md5_fill_dest_key
	.p2align 4,,15
.globl md5_run
	.type	md5_run, @function
md5_run:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$124, %esp
	movl	16(%ebp), %ebx
	movl	8(%ebp), %eax
	movl	%gs:20, %edx
	movl	%edx, -16(%ebp)
	xorl	%edx, %edx
	movl	12(%ebp), %esi
	cmpl	$63, %ebx
	movl	%ebx, %edi
	movl	%eax, -104(%ebp)
	jbe	.L63
	.p2align 4,,7
	.p2align 3
.L68:
	movl	-104(%ebp), %ecx
	subl	$64, %ebx
	movl	%esi, 4(%esp)
	movl	%ecx, (%esp)
	call	md5_process
	cmpl	$63, %ebx
	ja	.L68
	movl	16(%ebp), %edi
	andl	$63, %edi
.L63:
	testl	%edi, %edi
	je	.L66
	leal	-80(%ebp), %ebx
	movl	%edi, 8(%esp)
	movl	%esi, 4(%esp)
	movl	%ebx, (%esp)
	movl	$64, 12(%esp)
	call	__memcpy_chk
	movl	$64, %eax
	subl	%edi, %eax
	movl	16(%ebp), %edi
	movl	%eax, 8(%esp)
	movl	$0, 4(%esp)
	leal	(%ebx,%edi), %edx
	movl	%edx, (%esp)
	call	memset
	movl	-104(%ebp), %eax
	movl	%ebx, 4(%esp)
	movl	%eax, (%esp)
	call	md5_process
	movl	-104(%ebp), %ecx
	movl	-104(%ebp), %edx
	movl	4(%ecx), %esi
	movl	%ecx, %edi
	movl	(%edx), %edx
	movl	8(%ecx), %ecx
	movl	12(%edi), %ebx
	movl	%esi, %eax
	movl	%edx, -108(%ebp)
	movl	-108(%ebp), %edi
	notl	%eax
	movl	%ecx, %edx
	andl	%esi, %edx
	andl	%ebx, %eax
	orl	%edx, %eax
	leal	-546659208(%edi,%eax), %edx
	rorl	$25, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-389564586(%ebx,%eax), %ebx
	rorl	$20, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	606105819(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1044525330(%esi,%eax), %esi
	rorl	$10, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-176418897(%edx,%eax), %edx
	rorl	$25, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	1200080426(%ebx,%eax), %ebx
	rorl	$20, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-1473231341(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-45705983(%esi,%eax), %esi
	rorl	$10, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	1770035416(%edx,%eax), %edx
	rorl	$25, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1958414417(%ebx,%eax), %ebx
	rorl	$20, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-42063(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1990404162(%esi,%eax), %esi
	rorl	$10, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	1804603682(%edx,%eax), %edx
	rorl	$25, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-40341101(%ebx,%eax), %ebx
	rorl	$20, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	movl	%eax, -100(%ebp)
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-1502002290(%ecx,%eax), %ecx
	movl	16(%ebp), %eax
	rorl	$15, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %edi
	notl	%edi
	movl	%edi, -96(%ebp)
	leal	1236535329(%esi,%eax), %esi
	movl	%edi, %eax
	movl	%ecx, %edi
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %esi
	rorl	$10, %esi
	addl	%ecx, %esi
	andl	%ecx, -100(%ebp)
	movl	%esi, %eax
	andl	%ebx, %eax
	orl	-100(%ebp), %eax
	andl	%esi, -96(%ebp)
	leal	-165796510(%edx,%eax), %edx
	rorl	$27, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	andl	%ecx, %eax
	orl	-96(%ebp), %eax
	leal	-1069501632(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$23, %ebx
	notl	%eax
	addl	%edx, %ebx
	andl	%edx, %eax
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	643717713(%ecx,%edi), %ecx
	notl	%eax
	rorl	$18, %ecx
	addl	%ebx, %ecx
	andl	%ebx, %eax
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-239679574(%esi,%edi), %esi
	notl	%eax
	rorl	$12, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-701558691(%edx,%edi), %edx
	notl	%eax
	rorl	$27, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	38016083(%ebx,%edi), %ebx
	movl	16(%ebp), %edi
	notl	%eax
	rorl	$23, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	leal	-660478335(%ecx,%edi), %ecx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-405537848(%esi,%edi), %esi
	notl	%eax
	rorl	$12, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	568446438(%edx,%edi), %edx
	notl	%eax
	rorl	$27, %edx
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-1019803690(%ebx,%edi), %ebx
	notl	%eax
	rorl	$23, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-187363961(%ecx,%edi), %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	1163531501(%esi,%edi), %esi
	notl	%eax
	rorl	$12, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1444681467(%edx,%edi), %edx
	notl	%eax
	rorl	$27, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-51403784(%ebx,%edi), %ebx
	notl	%eax
	rorl	$23, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	1735328473(%ecx,%edi), %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1926607734(%esi,%edi), %esi
	movl	16(%ebp), %edi
	rorl	$12, %esi
	addl	%ecx, %esi
	xorl	%ebx, %eax
	xorl	%esi, %eax
	leal	-378558(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-2022574463(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$21, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	1839030562(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-35309556(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$9, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-1530992060(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	1272893353(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$21, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-155497632(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-1094730640(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$9, %esi
	addl	%ecx, %esi
	xorl	%ebx, %eax
	xorl	%esi, %eax
	leal	681279174(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-224319494(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$21, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-722521979(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	76029189(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$9, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-640364487(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-421815835(%ebx,%eax), %ebx
	movl	16(%ebp), %eax
	rorl	$21, %ebx
	addl	%edx, %ebx
	leal	530742520(%ecx,%eax), %ecx
	movl	%edx, %eax
	xorl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-995338651(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$9, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-64413116(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	1126891415(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$22, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-1416354905(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-57434055(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$11, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	1700485571(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	-1894986606(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$22, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-1051523(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-2054922799(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$11, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	1873313359(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	-30611744(%ebx,%edi), %ebx
	movl	-108(%ebp), %edi
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$22, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-1560198380(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	1309151649(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$11, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-145523070(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	-1120210379(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$22, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	718787259(%ecx,%eax), %ecx
	leal	(%edx,%edi), %eax
	movl	-104(%ebp), %edi
	notl	%edx
	rorl	$17, %ecx
	addl	%ebx, %ecx
	orl	%ecx, %edx
	xorl	%ebx, %edx
	movl	%eax, (%edi)
	movl	%ecx, %eax
	addl	4(%edi), %eax
	leal	-343485551(%esi,%edx), %esi
	rorl	$11, %esi
	addl	%esi, %eax
	addl	%ecx, 8(%edi)
	addl	%ebx, 12(%edi)
	movl	%eax, 4(%edi)
.L66:
	movl	-16(%ebp), %eax
	xorl	%gs:20, %eax
	jne	.L70
	addl	$124, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L70:
	call	__stack_chk_fail
	.size	md5_run, .-md5_run
	.section	.rodata.str1.4
	.align 4
.LC7:
	.string	"\n\nWARNING: Type-checked cast attempted on NULL pointer."
	.align 4
.LC8:
	.string	"\n\nLocation: %s, function %s, line %lu: "
	.text
	.p2align 4,,15
.globl _Ts_HashObject_NullErrorMessage
	.type	_Ts_HashObject_NullErrorMessage, @function
_Ts_HashObject_NullErrorMessage:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	stderr, %eax
	movl	$55, 8(%esp)
	movl	$1, 4(%esp)
	movl	$.LC7, (%esp)
	movl	%eax, 12(%esp)
	call	fwrite
	movl	16(%ebp), %eax
	movl	$.LC8, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, 20(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 16(%esp)
	movl	8(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	stderr, %eax
	movl	%eax, (%esp)
	call	__fprintf_chk
	leave
	ret
	.size	_Ts_HashObject_NullErrorMessage, .-_Ts_HashObject_NullErrorMessage
	.section	.rodata.str1.4
	.align 4
.LC9:
	.string	"\n\nERROR: Invalid upcast attempted from type "
	.section	.rodata.str1.1
.LC10:
	.string	"NULL TYPE"
.LC11:
	.string	"HashObject"
.LC12:
	.string	" to type %s."
	.section	.rodata.str1.4
	.align 4
.LC13:
	.string	"\nLocation: %s, function %s, line %lu: "
	.align 4
.LC14:
	.string	"\n PROGRAM MAY BECOME UNSTABLE."
	.text
	.p2align 4,,15
.globl _Ts_HashObject_UpCastErrorMessage
	.type	_Ts_HashObject_UpCastErrorMessage, @function
_Ts_HashObject_UpCastErrorMessage:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	movl	12(%ebp), %eax
	movl	%ebx, -12(%ebp)
	movl	8(%ebp), %ebx
	movl	%esi, -8(%ebp)
	movl	20(%ebp), %esi
	movl	%edi, -4(%ebp)
	movl	16(%ebp), %edi
	movl	%eax, -16(%ebp)
	movl	stderr, %eax
	movl	$44, 8(%esp)
	movl	$1, 4(%esp)
	movl	$.LC9, (%esp)
	movl	%eax, 12(%esp)
	call	fwrite
	movl	(%ebx), %edx
	testl	%edx, %edx
	je	.L77
	movl	stderr, %eax
	movl	%eax, 4(%esp)
	movl	(%edx), %eax
	movl	%eax, (%esp)
	call	fputs
.L75:
	movl	$.LC11, 12(%esp)
	movl	$.LC12, 8(%esp)
	movl	$1, 4(%esp)
	movl	stderr, %eax
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	%esi, 20(%esp)
	movl	%edi, 16(%esp)
	movl	-16(%ebp), %eax
	movl	$.LC13, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, 12(%esp)
	movl	stderr, %eax
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	stderr, %eax
	movl	-12(%ebp), %ebx
	movl	$30, 16(%ebp)
	movl	-8(%ebp), %esi
	movl	$1, 12(%ebp)
	movl	-4(%ebp), %edi
	movl	%eax, 20(%ebp)
	movl	$.LC14, 8(%ebp)
	movl	%ebp, %esp
	popl	%ebp
	jmp	fwrite
.L77:
	movl	stderr, %eax
	movl	$9, 8(%esp)
	movl	$1, 4(%esp)
	movl	$.LC10, (%esp)
	movl	%eax, 12(%esp)
	call	fwrite
	jmp	.L75
	.size	_Ts_HashObject_UpCastErrorMessage, .-_Ts_HashObject_UpCastErrorMessage
	.p2align 4,,15
.globl Hf_FillExact
	.type	Hf_FillExact, @function
Hf_FillExact:
	pushl	%ebp
	movl	$-1, %eax
	movl	%esp, %ebp
	movl	$64, %ecx
	pushl	%edi
	movl	$map.6880, %edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	rep stosl
	movl	8(%ebp), %eax
	movb	$0, map.6880+48
	movb	$1, map.6880+49
	movb	$2, map.6880+50
	testl	%eax, %eax
	movb	$3, map.6880+51
	movb	$4, map.6880+52
	movb	$5, map.6880+53
	movb	$6, map.6880+54
	movb	$7, map.6880+55
	movb	$8, map.6880+56
	movb	$9, map.6880+57
	movb	$10, map.6880+97
	movb	$11, map.6880+98
	movb	$12, map.6880+99
	movb	$13, map.6880+100
	movb	$14, map.6880+101
	movb	$15, map.6880+102
	movb	$10, map.6880+65
	movb	$11, map.6880+66
	movb	$12, map.6880+67
	movb	$13, map.6880+68
	movb	$14, map.6880+69
	movb	$15, map.6880+70
	movl	%eax, -16(%ebp)
	je	.L88
.L81:
	movl	$0, -24(%ebp)
	xorl	%ebx, %ebx
	xorl	%ecx, %ecx
	jmp	.L86
	.p2align 4,,7
	.p2align 3
.L84:
	addl	$1, -24(%ebp)
	cmpl	$32, -24(%ebp)
	je	.L85
.L89:
	movl	%edi, %ecx
.L86:
	leal	4(%ecx), %edi
	movl	-16(%ebp), %esi
	xorl	%eax, %eax
	movl	_bit_masks(,%edi,4), %edx
	subl	_bit_masks(,%ecx,4), %edx
	notl	%edx
	andl	8(%esi,%ebx,4), %edx
	cmpl	$31, %ecx
	movl	%edx, 8(%esi,%ebx,4)
	ja	.L83
	movl	12(%ebp), %esi
	movl	-24(%ebp), %eax
	movsbl	(%esi,%eax),%eax
	movl	%eax, -20(%ebp)
	movsbl	map.6880(%eax),%eax
	andl	$15, %eax
	sall	%cl, %eax
.L83:
	orl	%edx, %eax
	movl	-16(%ebp), %edx
	cmpl	$32, %edi
	movl	%eax, 8(%edx,%ebx,4)
	jne	.L84
	addl	$1, -24(%ebp)
	addl	$1, %ebx
	xorw	%di, %di
	cmpl	$32, -24(%ebp)
	jne	.L89
.L85:
	movl	-16(%ebp), %eax
	addl	$12, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L88:
	call	H_New
	movl	%eax, -16(%ebp)
	jmp	.L81
	.size	Hf_FillExact, .-Hf_FillExact
	.p2align 4,,15
.globl Hf_FromMarkedHashObject
	.type	Hf_FromMarkedHashObject, @function
Hf_FromMarkedHashObject:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$220, %esp
	movl	12(%ebp), %eax
	movl	8(%eax), %edx
	movl	12(%eax), %ecx
	movl	16(%eax), %ebx
	movl	20(%eax), %esi
	movl	28(%eax), %eax
	movl	%edx, -144(%ebp)
	movl	%edx, -76(%ebp)
	movl	%ecx, -148(%ebp)
	testl	%eax, %eax
	movl	%ecx, -72(%ebp)
	movl	%ebx, -152(%ebp)
	movl	%ebx, -68(%ebp)
	movl	%esi, -136(%ebp)
	movl	%esi, -64(%ebp)
	je	.L110
	movl	8(%eax), %edx
	testl	%edx, %edx
	movl	%edx, -140(%ebp)
	je	.L111
	movl	16(%eax), %eax
	leal	-44(%ebp), %ecx
	xorl	%ebx, %ebx
	xorl	%edx, %edx
	movl	%eax, -116(%ebp)
.L94:
	movl	-116(%ebp), %esi
	movl	$0, 4(%ecx)
	movl	$0, 12(%ecx)
	movl	(%esi,%edx,8), %eax
	movl	%eax, (%ecx)
	movl	4(%esi,%edx,8), %eax
	leal	1(%ebx), %edx
	movl	$1, %ebx
	movl	%eax, 8(%ecx)
	addl	$16, %ecx
	cmpl	%edx, -140(%ebp)
	jbe	.L102
	cmpl	$1, %edx
	jle	.L94
.L102:
	movl	-64(%ebp), %ecx
	movl	-76(%ebp), %edi
	movl	-72(%ebp), %eax
	movl	-60(%ebp), %ebx
	movl	%ecx, -176(%ebp)
	movl	-40(%ebp), %ecx
	movl	-56(%ebp), %esi
	movl	%edi, -164(%ebp)
	movl	-68(%ebp), %edx
	movl	%eax, -168(%ebp)
	movl	-48(%ebp), %eax
	movl	%ecx, -200(%ebp)
	movl	-16(%ebp), %ecx
	movl	%ebx, -180(%ebp)
	movl	-36(%ebp), %ebx
	movl	%esi, -184(%ebp)
	movl	-32(%ebp), %esi
	movl	%eax, -192(%ebp)
	movl	-24(%ebp), %eax
	movl	%ecx, -224(%ebp)
	movl	-164(%ebp), %ecx
	movl	%edx, -172(%ebp)
	movl	-52(%ebp), %edi
	movl	-44(%ebp), %edx
	movl	%ebx, -204(%ebp)
	movl	-168(%ebp), %ebx
	subl	$1433972890, %ecx
	rorl	$25, %ecx
	addl	$1560017565, %ecx
	movl	%esi, -208(%ebp)
	movl	%ecx, %esi
	movl	%eax, -216(%ebp)
	notl	%esi
	movl	%ecx, %eax
	andl	$1560017565, %eax
	andl	$1111091869, %esi
	movl	%edi, -188(%ebp)
	orl	%eax, %esi
	movl	-28(%ebp), %edi
	movl	%edx, -196(%ebp)
	movl	-20(%ebp), %edx
	leal	909491908(%esi,%ebx), %esi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%edi, -212(%ebp)
	movl	-172(%ebp), %edi
	movl	%esi, %eax
	movl	%edx, -220(%ebp)
	movl	%esi, %edx
	andl	%ecx, %eax
	notl	%edx
	andl	$1560017565, %edx
	orl	%eax, %edx
	leal	1717197688(%edx,%edi), %edx
	movl	-180(%ebp), %edi
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %ebx
	movl	%edx, %eax
	notl	%ebx
	andl	%esi, %eax
	andl	%ecx, %ebx
	orl	%eax, %ebx
	movl	-176(%ebp), %eax
	leal	-176418897(%ecx,%edi), %ecx
	leal	515492235(%ebx,%eax), %ebx
	rorl	$10, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-184(%ebp), %eax
	rorl	$25, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %edi
	leal	1200080426(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-188(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	leal	-1473231341(%edx,%edi), %edx
	notl	%eax
	movl	%esi, %edi
	andl	%ebx, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-192(%ebp), %eax
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %edi
	leal	-45705983(%ebx,%eax), %ebx
	movl	%edx, %eax
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	movl	-196(%ebp), %edi
	addl	%eax, %ebx
	rorl	$10, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	leal	1770035416(%ecx,%edi), %ecx
	notl	%eax
	movl	%ebx, %edi
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-200(%ebp), %eax
	rorl	$25, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %edi
	leal	-1958414417(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-204(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	leal	-42063(%edx,%edi), %edx
	notl	%eax
	movl	%esi, %edi
	andl	%ebx, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-208(%ebp), %eax
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %edi
	leal	-1990404162(%ebx,%eax), %ebx
	movl	%edx, %eax
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	movl	-212(%ebp), %edi
	addl	%eax, %ebx
	rorl	$10, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	leal	1804603682(%ecx,%edi), %ecx
	notl	%eax
	movl	%ebx, %edi
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-216(%ebp), %eax
	rorl	$25, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %edi
	leal	-40341101(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%esi, %edi
	notl	%edi
	movl	%edi, -92(%ebp)
	movl	-220(%ebp), %eax
	leal	-1502002290(%edx,%eax), %edx
	movl	%edi, %eax
	movl	%esi, %edi
	andl	%ebx, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %edx
	movl	-224(%ebp), %eax
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %edi
	notl	%edi
	movl	%edi, -88(%ebp)
	leal	1236535329(%ebx,%eax), %ebx
	movl	%edi, %eax
	movl	%edx, %edi
	andl	%ecx, %eax
	andl	%esi, %edi
	orl	%edi, %eax
	movl	-168(%ebp), %edi
	addl	%eax, %ebx
	rorl	$10, %ebx
	addl	%edx, %ebx
	andl	%edx, -92(%ebp)
	movl	%ebx, %eax
	andl	%esi, %eax
	orl	-92(%ebp), %eax
	leal	-165796510(%ecx,%edi), %ecx
	movl	-208(%ebp), %edi
	andl	%ebx, -88(%ebp)
	addl	%eax, %ecx
	movl	-188(%ebp), %eax
	rorl	$27, %ecx
	addl	%ebx, %ecx
	leal	-1069501632(%esi,%eax), %esi
	movl	%ecx, %eax
	andl	%edx, %eax
	orl	-88(%ebp), %eax
	leal	643717713(%edx,%edi), %edx
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%ecx, %esi
	andl	%ecx, %eax
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-164(%ebp), %eax
	addl	%edi, %edx
	rorl	$18, %edx
	addl	%esi, %edx
	leal	-373897302(%ebx,%eax), %ebx
	movl	%ecx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	movl	-184(%ebp), %edi
	notl	%eax
	rorl	$12, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	leal	-701558691(%ecx,%edi), %ecx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-204(%ebp), %eax
	addl	%edi, %ecx
	rorl	$27, %ecx
	addl	%ebx, %ecx
	leal	38016083(%esi,%eax), %esi
	movl	%edx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %esi
	movl	-224(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	leal	-660478335(%edx,%edi), %edx
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-180(%ebp), %eax
	addl	%edi, %edx
	rorl	$18, %edx
	addl	%esi, %edx
	leal	-405537848(%ebx,%eax), %ebx
	movl	%ecx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	movl	-200(%ebp), %edi
	notl	%eax
	rorl	$12, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	leal	568446438(%ecx,%edi), %ecx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-220(%ebp), %eax
	addl	%edi, %ecx
	rorl	$27, %ecx
	addl	%ebx, %ecx
	leal	-1019803690(%esi,%eax), %esi
	movl	%edx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %esi
	movl	-176(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	leal	-187363961(%edx,%edi), %edx
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-196(%ebp), %eax
	addl	%edi, %edx
	rorl	$18, %edx
	addl	%esi, %edx
	leal	1163531501(%ebx,%eax), %ebx
	movl	%ecx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	movl	-216(%ebp), %edi
	notl	%eax
	rorl	$12, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	leal	-1444681467(%ecx,%edi), %ecx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-172(%ebp), %eax
	addl	%edi, %ecx
	rorl	$27, %ecx
	addl	%ebx, %ecx
	leal	-51403784(%esi,%eax), %esi
	movl	%edx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %esi
	movl	-192(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	leal	1735328473(%edx,%edi), %edx
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-212(%ebp), %eax
	addl	%edi, %edx
	rorl	$18, %edx
	addl	%esi, %edx
	leal	-1926607734(%ebx,%eax), %ebx
	movl	%ecx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %ebx
	movl	-184(%ebp), %edi
	xorl	%esi, %eax
	rorl	$12, %ebx
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-378558(%ecx,%edi), %ecx
	movl	-208(%ebp), %edi
	addl	%eax, %ecx
	movl	-196(%ebp), %eax
	rorl	$28, %ecx
	addl	%ebx, %ecx
	leal	-2022574463(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	1839030562(%edx,%edi), %edx
	movl	-168(%ebp), %edi
	addl	%eax, %edx
	movl	-220(%ebp), %eax
	rorl	$16, %edx
	addl	%esi, %edx
	leal	-35309556(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$9, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-1530992060(%ecx,%edi), %ecx
	movl	-192(%ebp), %edi
	addl	%eax, %ecx
	movl	-180(%ebp), %eax
	rorl	$28, %ecx
	addl	%ebx, %ecx
	leal	1272893353(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-155497632(%edx,%edi), %edx
	movl	-216(%ebp), %edi
	addl	%eax, %edx
	movl	-204(%ebp), %eax
	rorl	$16, %edx
	addl	%esi, %edx
	leal	-1094730640(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$9, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	681279174(%ecx,%edi), %ecx
	movl	-176(%ebp), %edi
	addl	%eax, %ecx
	movl	-164(%ebp), %eax
	rorl	$28, %ecx
	addl	%ebx, %ecx
	leal	-358537222(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-722521979(%edx,%edi), %edx
	movl	-200(%ebp), %edi
	addl	%eax, %edx
	movl	-188(%ebp), %eax
	rorl	$16, %edx
	addl	%esi, %edx
	leal	76029189(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$9, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-640364487(%ecx,%edi), %ecx
	movl	-224(%ebp), %edi
	addl	%eax, %ecx
	movl	-212(%ebp), %eax
	rorl	$28, %ecx
	addl	%ebx, %ecx
	leal	-421815835(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	530742520(%edx,%edi), %edx
	movl	-164(%ebp), %edi
	addl	%eax, %edx
	movl	-172(%ebp), %eax
	rorl	$16, %edx
	addl	%esi, %edx
	leal	-995338651(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$9, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-198630844(%ecx,%edi), %ecx
	movl	-220(%ebp), %edi
	addl	%eax, %ecx
	movl	-192(%ebp), %eax
	rorl	$26, %ecx
	addl	%ebx, %ecx
	leal	1126891415(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-1416354905(%edx,%edi), %edx
	movl	-212(%ebp), %edi
	addl	%eax, %edx
	movl	-184(%ebp), %eax
	rorl	$17, %edx
	addl	%esi, %edx
	leal	-57434055(%ebx,%eax), %ebx
	movl	%ecx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$11, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	1700485571(%ecx,%edi), %ecx
	movl	-204(%ebp), %edi
	addl	%eax, %ecx
	movl	-176(%ebp), %eax
	rorl	$26, %ecx
	addl	%ebx, %ecx
	leal	-1894986606(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-1051523(%edx,%edi), %edx
	movl	-196(%ebp), %edi
	addl	%eax, %edx
	movl	-168(%ebp), %eax
	rorl	$17, %edx
	addl	%esi, %edx
	leal	-2054922799(%ebx,%eax), %ebx
	movl	%ecx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$11, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	1873313359(%ecx,%edi), %ecx
	movl	-188(%ebp), %edi
	addl	%eax, %ecx
	movl	-224(%ebp), %eax
	rorl	$26, %ecx
	addl	%ebx, %ecx
	leal	-30611744(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-1560198380(%edx,%edi), %edx
	movl	-180(%ebp), %edi
	addl	%eax, %edx
	movl	-216(%ebp), %eax
	rorl	$17, %edx
	addl	%esi, %edx
	leal	1309151649(%ebx,%eax), %ebx
	movl	%ecx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$11, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-145523070(%ecx,%edi), %ecx
	movl	-172(%ebp), %edi
	addl	%eax, %ecx
	movl	-208(%ebp), %eax
	rorl	$26, %ecx
	addl	%ebx, %ecx
	leal	-1120210379(%esi,%eax), %esi
	movl	%edx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	718787259(%edx,%edi), %edx
	addl	%eax, %edx
	leal	-1847674129(%ecx), %eax
	notl	%ecx
	movl	%eax, -120(%ebp)
	movl	-200(%ebp), %edi
	rorl	$17, %edx
	addl	%esi, %edx
	orl	%edx, %ecx
	xorl	%esi, %ecx
	addl	$1299056494, %esi
	leal	-343485551(%ebx,%edi), %ebx
	addl	%ecx, %ebx
	rorl	$11, %ebx
	leal	1560017565(%edx,%ebx), %ebx
	addl	$1111091869, %edx
	cmpl	$2, -140(%ebp)
	movl	%edx, -128(%ebp)
	movl	$2, %edx
	movl	%ebx, -124(%ebp)
	movl	%esi, -132(%ebp)
	movl	$6, -112(%ebp)
	ja	.L96
.L92:
	movl	8(%ebp), %eax
	testl	%eax, %eax
	je	.L112
.L100:
	movl	-128(%ebp), %ebx
	movl	-132(%ebp), %esi
	movl	-120(%ebp), %edx
	movl	-124(%ebp), %ecx
	movl	%ebx, 16(%eax)
	movl	%esi, 20(%eax)
	movl	%edx, 8(%eax)
	movl	%ecx, 12(%eax)
	addl	$220, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L111:
	movl	-144(%ebp), %edx
	movl	(%eax), %ecx
	movl	4(%eax), %eax
	movl	-148(%ebp), %ebx
	subl	$1433972890, %edx
	movl	-152(%ebp), %edi
	rorl	$25, %edx
	addl	$1560017565, %edx
	movl	%eax, -160(%ebp)
	movl	%eax, -52(%ebp)
	movl	%edx, %eax
	movl	%ecx, -156(%ebp)
	notl	%eax
	movl	%ecx, -60(%ebp)
	movl	%edx, %ecx
	andl	$1111091869, %eax
	andl	$1560017565, %ecx
	orl	%ecx, %eax
	leal	909491908(%ebx,%eax), %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %ecx
	notl	%eax
	andl	%edx, %ecx
	andl	$1560017565, %eax
	orl	%ecx, %eax
	leal	1717197688(%edi,%eax), %ecx
	movl	-136(%ebp), %edi
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %ebx
	notl	%eax
	andl	%esi, %ebx
	andl	%edx, %eax
	orl	%ebx, %eax
	leal	515492235(%edi,%eax), %ebx
	movl	-156(%ebp), %eax
	rorl	$10, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %edi
	leal	-176418897(%edx,%eax), %edx
	movl	%ebx, %eax
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	addl	%eax, %edx
	rorl	$25, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	movl	-160(%ebp), %edi
	leal	1200080426(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	leal	-1473231341(%ecx,%edi), %ecx
	notl	%eax
	movl	%esi, %edi
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-45705983(%ebx,%eax), %ebx
	rorl	$10, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1770035416(%edx,%eax), %edx
	rorl	$25, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1958414417(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-42063(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1990404162(%ebx,%eax), %ebx
	rorl	$10, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1804603682(%edx,%eax), %edx
	rorl	$25, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	leal	-40341101(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %edi
	movl	%eax, -100(%ebp)
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-1502002290(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %edi
	notl	%edi
	movl	%edi, -96(%ebp)
	movl	%edi, %eax
	movl	%ecx, %edi
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	movl	-160(%ebp), %edi
	leal	1236535329(%ebx,%eax), %ebx
	movl	-148(%ebp), %eax
	rorl	$10, %ebx
	addl	%ecx, %ebx
	andl	%ecx, -100(%ebp)
	leal	-165796510(%edx,%eax), %edx
	movl	%ebx, %eax
	andl	%esi, %eax
	orl	-100(%ebp), %eax
	andl	%ebx, -96(%ebp)
	leal	-1069501632(%esi,%edi), %esi
	addl	%eax, %edx
	rorl	$27, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	andl	%ecx, %eax
	orl	-96(%ebp), %eax
	movl	$0, -56(%ebp)
	movl	$0, -48(%ebp)
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%edx, %esi
	andl	%edx, %eax
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-144(%ebp), %eax
	leal	643717713(%ecx,%edi), %ecx
	rorl	$18, %ecx
	addl	%esi, %ecx
	leal	-373897302(%ebx,%eax), %ebx
	movl	%edx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	notl	%eax
	rorl	$12, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-701558691(%edx,%edi), %edx
	notl	%eax
	rorl	$27, %edx
	andl	%ebx, %eax
	addl	%ebx, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	38016083(%esi,%edi), %esi
	notl	%eax
	rorl	$23, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-660478335(%ecx,%edi), %ecx
	movl	-156(%ebp), %edi
	notl	%eax
	rorl	$18, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	leal	-405537848(%ebx,%edi), %ebx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	notl	%eax
	rorl	$12, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	568446438(%edx,%edi), %edx
	notl	%eax
	rorl	$27, %edx
	andl	%ebx, %eax
	addl	%ebx, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-136(%ebp), %eax
	leal	-1019803690(%esi,%edi), %esi
	rorl	$23, %esi
	addl	%edx, %esi
	leal	-187363961(%ecx,%eax), %ecx
	movl	%ebx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	1163531501(%ebx,%edi), %ebx
	notl	%eax
	rorl	$12, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1444681467(%edx,%edi), %edx
	movl	-152(%ebp), %edi
	notl	%eax
	rorl	$27, %edx
	andl	%ebx, %eax
	addl	%ebx, %edx
	leal	-51403784(%esi,%edi), %esi
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %esi
	notl	%eax
	rorl	$23, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	1735328473(%ecx,%edi), %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1926607734(%ebx,%edi), %ebx
	movl	-156(%ebp), %edi
	rorl	$12, %ebx
	addl	%ecx, %ebx
	xorl	%esi, %eax
	xorl	%ebx, %eax
	leal	-378558(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	-2022574463(%esi,%eax), %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	1839030562(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	-35309556(%ebx,%eax), %ebx
	movl	-148(%ebp), %eax
	rorl	$9, %ebx
	addl	%ecx, %ebx
	leal	-1530992060(%edx,%eax), %edx
	movl	%ecx, %eax
	xorl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %edx
	movl	%ebx, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	1272893353(%esi,%edi), %esi
	movl	-136(%ebp), %edi
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	-155497632(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	xorl	%edx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	-1094730640(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$9, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	681279174(%edx,%eax), %edx
	movl	-144(%ebp), %eax
	rorl	$28, %edx
	addl	%ebx, %edx
	leal	-358537222(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	-722521979(%ecx,%edi), %ecx
	movl	-152(%ebp), %edi
	addl	%eax, %ecx
	movl	-160(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	76029189(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %ebx
	movl	%ecx, %eax
	rorl	$9, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	-640364487(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$28, %edx
	xorl	%ecx, %eax
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	-421815835(%esi,%eax), %esi
	movl	%edx, %eax
	rorl	$21, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	530742520(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	xorl	%edx, %eax
	xorl	%ecx, %eax
	leal	-995338651(%ebx,%edi), %ebx
	movl	-136(%ebp), %edi
	addl	%eax, %ebx
	movl	-144(%ebp), %eax
	rorl	$9, %ebx
	addl	%ecx, %ebx
	leal	-198630844(%edx,%eax), %edx
	movl	%esi, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	1126891415(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1416354905(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-57434055(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$11, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	1700485571(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	-1894986606(%esi,%edi), %esi
	movl	-160(%ebp), %edi
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1051523(%ecx,%eax), %ecx
	movl	-148(%ebp), %eax
	rorl	$17, %ecx
	addl	%esi, %ecx
	leal	-2054922799(%ebx,%eax), %ebx
	movl	%edx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$11, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	1873313359(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	-30611744(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1560198380(%ecx,%edi), %ecx
	movl	-152(%ebp), %edi
	addl	%eax, %ecx
	movl	%edx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	1309151649(%ebx,%eax), %ebx
	movl	-156(%ebp), %eax
	rorl	$11, %ebx
	addl	%ecx, %ebx
	leal	-145523070(%edx,%eax), %edx
	movl	%esi, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$26, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	-1120210379(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	718787259(%ecx,%edi), %ecx
	addl	%eax, %ecx
	rorl	$17, %ecx
	addl	%esi, %ecx
	leal	-1847674129(%edx), %eax
	notl	%edx
	orl	%ecx, %edx
	xorl	%esi, %edx
	addl	$1299056494, %esi
	leal	-343485551(%ebx,%edx), %ebx
	rorl	$11, %ebx
	leal	1560017565(%ecx,%ebx), %ebx
	addl	$1111091869, %ecx
	movl	%eax, -120(%ebp)
	movl	%ebx, -124(%ebp)
	movl	%ecx, -128(%ebp)
	movl	%esi, -132(%ebp)
	jmp	.L92
	.p2align 4,,7
	.p2align 3
.L110:
	movl	%edx, %ebx
	movl	%ecx, %edi
	subl	$1433972890, %ebx
	rorl	$25, %ebx
	leal	1560017565(%ebx), %ecx
	movl	%ecx, %eax
	movl	%ecx, %edx
	notl	%eax
	andl	$1560017565, %edx
	andl	$1111091869, %eax
	orl	%edx, %eax
	leal	909491908(%edi,%eax), %esi
	movl	-152(%ebp), %edi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edx
	notl	%eax
	andl	%ecx, %edx
	andl	$1560017565, %eax
	orl	%edx, %eax
	leal	1717197688(%edi,%eax), %edx
	movl	-136(%ebp), %edi
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	notl	%eax
	andl	%ecx, %eax
	movl	%edx, %ecx
	andl	%esi, %ecx
	orl	%ecx, %eax
	leal	515492235(%edi,%eax), %ecx
	rorl	$10, %ecx
	addl	%edx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1383598668(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	1200080426(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1473231341(%edx,%eax), %edx
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%esi, %edi
	orl	%edi, %eax
	leal	-45705983(%ecx,%eax), %ecx
	rorl	$10, %ecx
	addl	%edx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1770035416(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1958414417(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-42063(%edx,%eax), %edx
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-1990404162(%ecx,%eax), %ecx
	rorl	$10, %ecx
	addl	%edx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1804603682(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-40341101(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %edi
	movl	%eax, -108(%ebp)
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1502002290(%edx,%eax), %edx
	rorl	$15, %edx
	addl	%esi, %edx
	movl	%edx, %edi
	notl	%edi
	movl	%edi, -104(%ebp)
	movl	%edi, %eax
	movl	%edx, %edi
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	1236535329(%ecx,%eax), %ecx
	movl	-148(%ebp), %eax
	rorl	$10, %ecx
	addl	%edx, %ecx
	andl	%edx, -108(%ebp)
	leal	-165796510(%ebx,%eax), %ebx
	movl	%ecx, %eax
	andl	%esi, %eax
	orl	-108(%ebp), %eax
	andl	%ecx, -104(%ebp)
	addl	%eax, %ebx
	rorl	$27, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	andl	%edx, %eax
	orl	-104(%ebp), %eax
	leal	-1069501632(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%ebx, %esi
	andl	%ebx, %eax
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	643717713(%edx,%edi), %edx
	movl	-144(%ebp), %edi
	notl	%eax
	rorl	$18, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	leal	-373897302(%ecx,%edi), %ecx
	movl	%edx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$12, %ecx
	addl	%edx, %ecx
	movl	%ecx, %edi
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-701558691(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	38016083(%esi,%edi), %esi
	notl	%eax
	rorl	$23, %esi
	andl	%ebx, %eax
	addl	%ebx, %esi
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-660478335(%edx,%edi), %edx
	notl	%eax
	rorl	$18, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-405537848(%ecx,%edi), %ecx
	notl	%eax
	rorl	$12, %ecx
	andl	%edx, %eax
	addl	%edx, %ecx
	movl	%ecx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	568446438(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	-136(%ebp), %eax
	leal	-1019803690(%esi,%edi), %esi
	rorl	$23, %esi
	addl	%ebx, %esi
	leal	-187363961(%edx,%eax), %edx
	movl	%ecx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %edx
	notl	%eax
	rorl	$18, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	1163531501(%ecx,%edi), %ecx
	notl	%eax
	rorl	$12, %ecx
	andl	%edx, %eax
	addl	%edx, %ecx
	movl	%ecx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-1444681467(%ebx,%edi), %ebx
	movl	-152(%ebp), %edi
	notl	%eax
	rorl	$27, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	leal	-51403784(%esi,%edi), %esi
	movl	%ebx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %esi
	notl	%eax
	rorl	$23, %esi
	andl	%ebx, %eax
	addl	%ebx, %esi
	movl	%esi, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	1735328473(%edx,%edi), %edx
	notl	%eax
	rorl	$18, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-1926607734(%ecx,%edi), %ecx
	xorl	%esi, %eax
	movl	-144(%ebp), %edi
	rorl	$12, %ecx
	addl	%edx, %ecx
	xorl	%ecx, %eax
	leal	-378558(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$28, %ebx
	addl	%ecx, %ebx
	xorl	%edx, %eax
	xorl	%ebx, %eax
	leal	-2022574463(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	1839030562(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$16, %edx
	xorl	%ebx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-35309556(%ecx,%eax), %ecx
	movl	-148(%ebp), %eax
	rorl	$9, %ecx
	addl	%edx, %ecx
	leal	-1530992060(%ebx,%eax), %ebx
	movl	%edx, %eax
	xorl	%esi, %eax
	xorl	%ecx, %eax
	addl	%eax, %ebx
	movl	%ecx, %eax
	rorl	$28, %ebx
	xorl	%edx, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	1272893353(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	-155497632(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$16, %edx
	xorl	%ebx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-1094730640(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$9, %ecx
	xorl	%esi, %eax
	addl	%edx, %ecx
	xorl	%ecx, %eax
	leal	681279174(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$28, %ebx
	addl	%ecx, %ebx
	xorl	%edx, %eax
	xorl	%ebx, %eax
	leal	-358537222(%esi,%edi), %esi
	movl	-152(%ebp), %edi
	addl	%eax, %esi
	movl	-136(%ebp), %eax
	rorl	$21, %esi
	addl	%ebx, %esi
	leal	-722521979(%edx,%eax), %edx
	movl	%ebx, %eax
	xorl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$16, %edx
	xorl	%ebx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	76029189(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$9, %ecx
	xorl	%esi, %eax
	addl	%edx, %ecx
	xorl	%ecx, %eax
	leal	-640364487(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$28, %ebx
	xorl	%edx, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	-421815835(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%ecx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	530742520(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$16, %edx
	xorl	%ebx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-995338651(%ecx,%edi), %ecx
	movl	-136(%ebp), %edi
	addl	%eax, %ecx
	movl	-144(%ebp), %eax
	rorl	$9, %ecx
	addl	%edx, %ecx
	leal	-198630844(%ebx,%eax), %ebx
	movl	%esi, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	1126891415(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1416354905(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$17, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	-57434055(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%edx, %ecx
	orl	%ecx, %eax
	xorl	%edx, %eax
	leal	1700485571(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	-1894986606(%esi,%edi), %esi
	movl	-152(%ebp), %edi
	addl	%eax, %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1051523(%edx,%eax), %edx
	movl	-148(%ebp), %eax
	rorl	$17, %edx
	addl	%esi, %edx
	leal	-2054922799(%ecx,%eax), %ecx
	movl	%ebx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%edx, %ecx
	orl	%ecx, %eax
	xorl	%edx, %eax
	leal	1873313359(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	-30611744(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1560198380(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$17, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	1309151649(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$11, %ecx
	notl	%eax
	addl	%edx, %ecx
	orl	%ecx, %eax
	xorl	%edx, %eax
	leal	-145523070(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	-1120210379(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	718787259(%edx,%edi), %edx
	addl	%eax, %edx
	rorl	$17, %edx
	addl	%esi, %edx
	leal	-1847674129(%ebx), %eax
	notl	%ebx
	orl	%edx, %ebx
	xorl	%esi, %ebx
	leal	-343485551(%ecx,%ebx), %ecx
	rorl	$11, %ecx
	leal	1560017565(%edx,%ecx), %ecx
	addl	$1111091869, %edx
	addl	$1299056494, %esi
	movl	%eax, -120(%ebp)
	movl	%ecx, -124(%ebp)
	movl	%edx, -128(%ebp)
	movl	%esi, -132(%ebp)
	jmp	.L92
	.p2align 4,,7
	.p2align 3
.L112:
	call	H_New
	jmp	.L100
	.p2align 4,,7
	.p2align 3
.L107:
	leal	2(%edx), %eax
	movl	%ebx, %ecx
	movl	(%ebx,%eax,4), %edi
	cmpl	%eax, -140(%ebp)
	movl	%esi, -60(%ebp)
	movl	$0, -56(%ebp)
	movl	%edi, -52(%ebp)
	movl	$0, -48(%ebp)
	jbe	.L113
	leal	3(%edx), %eax
	movl	(%ebx,%eax,4), %ebx
	cmpl	%eax, -140(%ebp)
	movl	%edi, -44(%ebp)
	movl	$0, -40(%ebp)
	movl	%ebx, -36(%ebp)
	movl	$0, -32(%ebp)
	jbe	.L114
	movl	16(%ecx,%edx,4), %edx
	movl	-232(%ebp), %ecx
	movl	%ebx, -28(%ebp)
	movl	$0, -24(%ebp)
	movl	%edx, -20(%ebp)
	movl	$0, -16(%ebp)
	movl	%ecx, -164(%ebp)
	movl	$0, -168(%ebp)
	movl	%esi, -172(%ebp)
	movl	$0, -176(%ebp)
	movl	%esi, -180(%ebp)
	movl	$0, -184(%ebp)
	movl	%edi, -188(%ebp)
	movl	$0, -192(%ebp)
	movl	%edi, -196(%ebp)
	movl	$0, -200(%ebp)
	movl	%ebx, -204(%ebp)
	movl	$0, -208(%ebp)
	movl	%ebx, -212(%ebp)
	movl	$0, -216(%ebp)
	movl	%edx, -220(%ebp)
	movl	$0, -224(%ebp)
.L97:
	movl	-124(%ebp), %eax
	movl	-120(%ebp), %edi
	movl	-164(%ebp), %esi
	movl	-124(%ebp), %edx
	andl	-128(%ebp), %edx
	notl	%eax
	andl	-132(%ebp), %eax
	leal	-680876936(%edi,%esi), %ebx
	movl	-172(%ebp), %edi
	orl	%edx, %eax
	movl	-132(%ebp), %edx
	addl	%eax, %ebx
	movl	-168(%ebp), %eax
	rorl	$25, %ebx
	addl	-124(%ebp), %ebx
	leal	-389564586(%edx,%eax), %esi
	movl	-124(%ebp), %edx
	movl	%ebx, %eax
	notl	%eax
	andl	-128(%ebp), %eax
	andl	%ebx, %edx
	orl	%edx, %eax
	addl	%eax, %esi
	movl	-128(%ebp), %eax
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %edx
	leal	606105819(%eax,%edi), %ecx
	movl	%esi, %eax
	movl	-176(%ebp), %edi
	notl	%eax
	andl	%ebx, %edx
	andl	-124(%ebp), %eax
	orl	%edx, %eax
	addl	%eax, %ecx
	movl	-124(%ebp), %eax
	rorl	$15, %ecx
	addl	%esi, %ecx
	leal	-1044525330(%eax,%edi), %edx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	movl	-180(%ebp), %edi
	addl	%eax, %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	movl	%edx, %eax
	leal	-176418897(%ebx,%edi), %ebx
	notl	%eax
	movl	%edx, %edi
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-184(%ebp), %eax
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %edi
	leal	1200080426(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	andl	%edx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	movl	-188(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	leal	-1473231341(%ecx,%edi), %ecx
	notl	%eax
	movl	%esi, %edi
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-192(%ebp), %eax
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %edi
	leal	-45705983(%edx,%eax), %edx
	movl	%ecx, %eax
	notl	%eax
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	movl	-196(%ebp), %edi
	addl	%eax, %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	movl	%edx, %eax
	leal	1770035416(%ebx,%edi), %ebx
	notl	%eax
	movl	%edx, %edi
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-200(%ebp), %eax
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %edi
	leal	-1958414417(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	andl	%edx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	movl	-204(%ebp), %edi
	addl	%eax, %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	leal	-42063(%ecx,%edi), %ecx
	notl	%eax
	movl	%esi, %edi
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-208(%ebp), %eax
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %edi
	leal	-1990404162(%edx,%eax), %edx
	movl	%ecx, %eax
	notl	%eax
	andl	%ebx, %eax
	andl	%esi, %edi
	orl	%edi, %eax
	movl	-212(%ebp), %edi
	addl	%eax, %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	movl	%edx, %eax
	leal	1804603682(%ebx,%edi), %ebx
	notl	%eax
	movl	%edx, %edi
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	addl	%eax, %ebx
	movl	-216(%ebp), %eax
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %edi
	leal	-40341101(%esi,%eax), %esi
	movl	%ebx, %eax
	notl	%eax
	andl	%edx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	addl	%eax, %esi
	movl	-220(%ebp), %eax
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %edi
	notl	%edi
	movl	%edi, -84(%ebp)
	leal	-1502002290(%ecx,%eax), %ecx
	movl	%edi, %eax
	movl	%esi, %edi
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	addl	%eax, %ecx
	movl	-224(%ebp), %eax
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %edi
	notl	%edi
	movl	%edi, -80(%ebp)
	leal	1236535329(%edx,%eax), %edx
	movl	%edi, %eax
	movl	%ecx, %edi
	andl	%ebx, %eax
	andl	%esi, %edi
	orl	%edi, %eax
	movl	-168(%ebp), %edi
	addl	%eax, %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	andl	%ecx, -84(%ebp)
	movl	%edx, %eax
	andl	%esi, %eax
	orl	-84(%ebp), %eax
	leal	-165796510(%ebx,%edi), %ebx
	addl	%eax, %ebx
	movl	-188(%ebp), %eax
	rorl	$27, %ebx
	addl	%edx, %ebx
	andl	%edx, -80(%ebp)
	leal	-1069501632(%esi,%eax), %esi
	movl	%ebx, %eax
	movl	-208(%ebp), %edi
	andl	%ecx, %eax
	orl	-80(%ebp), %eax
	leal	643717713(%ecx,%edi), %ecx
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%ebx, %esi
	andl	%ebx, %eax
	movl	%esi, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	-164(%ebp), %eax
	addl	%edi, %ecx
	rorl	$18, %ecx
	addl	%esi, %ecx
	leal	-373897302(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %edx
	movl	-184(%ebp), %edi
	notl	%eax
	rorl	$12, %edx
	andl	%ecx, %eax
	addl	%ecx, %edx
	leal	-701558691(%ebx,%edi), %ebx
	movl	%edx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-204(%ebp), %eax
	addl	%edi, %ebx
	rorl	$27, %ebx
	addl	%edx, %ebx
	leal	38016083(%esi,%eax), %esi
	movl	%ecx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %esi
	movl	-224(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	addl	%ebx, %esi
	leal	-660478335(%ecx,%edi), %ecx
	movl	%esi, %edi
	andl	%edx, %edi
	andl	%ebx, %eax
	orl	%eax, %edi
	movl	-180(%ebp), %eax
	addl	%edi, %ecx
	rorl	$18, %ecx
	addl	%esi, %ecx
	leal	-405537848(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %edx
	movl	-200(%ebp), %edi
	notl	%eax
	rorl	$12, %edx
	andl	%ecx, %eax
	addl	%ecx, %edx
	leal	568446438(%ebx,%edi), %ebx
	movl	%edx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-220(%ebp), %eax
	addl	%edi, %ebx
	rorl	$27, %ebx
	addl	%edx, %ebx
	leal	-1019803690(%esi,%eax), %esi
	movl	%ecx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %esi
	movl	-176(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%ebx, %eax
	addl	%ebx, %esi
	leal	-187363961(%ecx,%edi), %ecx
	movl	%esi, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	-196(%ebp), %eax
	addl	%edi, %ecx
	rorl	$18, %ecx
	addl	%esi, %ecx
	leal	1163531501(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %edx
	movl	-216(%ebp), %edi
	notl	%eax
	rorl	$12, %edx
	andl	%ecx, %eax
	addl	%ecx, %edx
	leal	-1444681467(%ebx,%edi), %ebx
	movl	%edx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	-172(%ebp), %eax
	addl	%edi, %ebx
	rorl	$27, %ebx
	addl	%edx, %ebx
	leal	-51403784(%esi,%eax), %esi
	movl	%ecx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %eax
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %esi
	movl	-192(%ebp), %edi
	notl	%eax
	rorl	$23, %esi
	andl	%ebx, %eax
	addl	%ebx, %esi
	leal	1735328473(%ecx,%edi), %ecx
	movl	%esi, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	-212(%ebp), %eax
	addl	%edi, %ecx
	rorl	$18, %ecx
	addl	%esi, %ecx
	leal	-1926607734(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %edx
	movl	-184(%ebp), %edi
	xorl	%esi, %eax
	rorl	$12, %edx
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	-378558(%ebx,%edi), %ebx
	movl	-208(%ebp), %edi
	addl	%eax, %ebx
	movl	-196(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	-2022574463(%esi,%eax), %esi
	movl	%edx, %eax
	xorl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	1839030562(%ecx,%edi), %ecx
	movl	-168(%ebp), %edi
	addl	%eax, %ecx
	movl	-220(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	-35309556(%edx,%eax), %edx
	movl	%esi, %eax
	xorl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$9, %edx
	xorl	%esi, %eax
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	-1530992060(%ebx,%edi), %ebx
	movl	-192(%ebp), %edi
	addl	%eax, %ebx
	movl	-180(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	1272893353(%esi,%eax), %esi
	movl	%edx, %eax
	xorl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	-155497632(%ecx,%edi), %ecx
	movl	-216(%ebp), %edi
	addl	%eax, %ecx
	movl	-204(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	-1094730640(%edx,%eax), %edx
	movl	%esi, %eax
	xorl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$9, %edx
	addl	%ecx, %edx
	xorl	%esi, %eax
	xorl	%edx, %eax
	leal	681279174(%ebx,%edi), %ebx
	movl	-176(%ebp), %edi
	addl	%eax, %ebx
	movl	-164(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	-358537222(%esi,%eax), %esi
	movl	%edx, %eax
	xorl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	-722521979(%ecx,%edi), %ecx
	movl	-200(%ebp), %edi
	addl	%eax, %ecx
	movl	-188(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	76029189(%edx,%eax), %edx
	movl	%esi, %eax
	xorl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$9, %edx
	xorl	%esi, %eax
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	-640364487(%ebx,%edi), %ebx
	movl	-224(%ebp), %edi
	addl	%eax, %ebx
	movl	-212(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	-421815835(%esi,%eax), %esi
	movl	%edx, %eax
	xorl	%ecx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	530742520(%ecx,%edi), %ecx
	movl	-164(%ebp), %edi
	addl	%eax, %ecx
	movl	-172(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	-995338651(%edx,%eax), %edx
	movl	%esi, %eax
	xorl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$9, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	-198630844(%ebx,%edi), %ebx
	movl	-220(%ebp), %edi
	addl	%eax, %ebx
	movl	-192(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	1126891415(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1416354905(%ecx,%edi), %ecx
	movl	-212(%ebp), %edi
	addl	%eax, %ecx
	movl	-184(%ebp), %eax
	rorl	$17, %ecx
	addl	%esi, %ecx
	leal	-57434055(%edx,%eax), %edx
	movl	%ebx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	1700485571(%ebx,%edi), %ebx
	movl	-204(%ebp), %edi
	addl	%eax, %ebx
	movl	-176(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	-1894986606(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1051523(%ecx,%edi), %ecx
	movl	-196(%ebp), %edi
	addl	%eax, %ecx
	movl	-168(%ebp), %eax
	rorl	$17, %ecx
	addl	%esi, %ecx
	leal	-2054922799(%edx,%eax), %edx
	movl	%ebx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	1873313359(%ebx,%edi), %ebx
	movl	-188(%ebp), %edi
	addl	%eax, %ebx
	movl	-224(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	-30611744(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1560198380(%ecx,%edi), %ecx
	movl	-180(%ebp), %edi
	addl	%eax, %ecx
	movl	-216(%ebp), %eax
	rorl	$17, %ecx
	addl	%esi, %ecx
	leal	1309151649(%edx,%eax), %edx
	movl	%ebx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	-145523070(%ebx,%edi), %ebx
	movl	-172(%ebp), %edi
	addl	%eax, %ebx
	movl	-208(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	-1120210379(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	718787259(%ecx,%edi), %ecx
	addl	%eax, %ecx
	movl	-200(%ebp), %eax
	rorl	$17, %ecx
	addl	%esi, %ecx
	addl	%ebx, -120(%ebp)
	leal	-343485551(%edx,%eax), %edx
	movl	%ebx, %eax
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	addl	%eax, %edx
	movl	-112(%ebp), %eax
	rorl	$11, %edx
	leal	(%ecx,%edx), %edx
	addl	%edx, -124(%ebp)
	movl	-112(%ebp), %edx
	addl	$4, %eax
	addl	%ecx, -128(%ebp)
	addl	%esi, -132(%ebp)
	cmpl	%edx, -140(%ebp)
	jbe	.L92
	movl	%eax, -112(%ebp)
.L96:
	cmpl	%edx, -140(%ebp)
	jbe	.L97
	movl	-116(%ebp), %ecx
	leal	1(%edx), %eax
	movl	-116(%ebp), %ebx
	cmpl	%eax, -140(%ebp)
	movl	$0, -72(%ebp)
	movl	(%ecx,%edx,4), %ecx
	movl	(%ebx,%eax,4), %esi
	movl	$0, -64(%ebp)
	movl	%ecx, -232(%ebp)
	movl	%ecx, -76(%ebp)
	movl	%esi, -68(%ebp)
	ja	.L107
	movl	%ecx, -164(%ebp)
	movl	$0, -168(%ebp)
	movl	%esi, -172(%ebp)
	movl	$0, -176(%ebp)
	jmp	.L97
.L113:
	movl	-232(%ebp), %ebx
	movl	$0, -168(%ebp)
	movl	%esi, -172(%ebp)
	movl	$0, -176(%ebp)
	movl	%ebx, -164(%ebp)
	movl	%esi, -180(%ebp)
	movl	$0, -184(%ebp)
	movl	%edi, -188(%ebp)
	movl	$0, -192(%ebp)
	jmp	.L97
.L114:
	movl	-232(%ebp), %eax
	movl	$0, -168(%ebp)
	movl	%esi, -172(%ebp)
	movl	$0, -176(%ebp)
	movl	%eax, -164(%ebp)
	movl	%esi, -180(%ebp)
	movl	$0, -184(%ebp)
	movl	%edi, -188(%ebp)
	movl	$0, -192(%ebp)
	movl	%edi, -196(%ebp)
	movl	$0, -200(%ebp)
	movl	%ebx, -204(%ebp)
	movl	$0, -208(%ebp)
	jmp	.L97
	.size	Hf_FromMarkedHashObject, .-Hf_FromMarkedHashObject
	.p2align 4,,15
.globl _Hf_FromUInt
	.type	_Hf_FromUInt, @function
_Hf_FromUInt:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$28, %esp
	movl	16(%ebp), %ebx
	subl	$971780855, %ebx
	rorl	$25, %ebx
	addl	$1013567884, %ebx
	movl	%ebx, %ecx
	movl	%ebx, %eax
	notl	%ecx
	andl	$1013567884, %eax
	andl	$-1386958736, %ecx
	orl	%eax, %ecx
	subl	$320936488, %ecx
	rorl	$20, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %esi
	movl	%ecx, %eax
	notl	%esi
	andl	%ebx, %eax
	andl	$1013567884, %esi
	orl	%eax, %esi
	subl	$780852917, %esi
	rorl	$15, %esi
	addl	%ecx, %esi
	movl	%esi, %edx
	movl	%esi, %eax
	notl	%edx
	andl	%ecx, %eax
	andl	%ebx, %edx
	orl	%eax, %edx
	movl	12(%ebp), %eax
	subl	$30957446, %edx
	rorl	$10, %edx
	addl	%esi, %edx
	leal	-176418897(%ebx,%eax), %ebx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	addl	%eax, %ebx
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1200080426(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%edx, %eax
	andl	%ebx, %edi
	orl	%edi, %eax
	leal	-1473231341(%esi,%eax), %esi
	rorl	$15, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-45705983(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	1770035416(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-1958414417(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-42063(%esi,%eax), %esi
	rorl	$15, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-1990404162(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%esi, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	1804603682(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-40341101(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%ebx, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%ebx, %edi
	movl	%eax, -28(%ebp)
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1502002290(%esi,%eax), %esi
	rorl	$15, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %edi
	movl	%eax, -24(%ebp)
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	1236535329(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%esi, %edx
	andl	%esi, -28(%ebp)
	movl	%edx, %eax
	andl	%ecx, %eax
	orl	-28(%ebp), %eax
	andl	%edx, -24(%ebp)
	leal	-165796510(%ebx,%eax), %ebx
	rorl	$27, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	andl	%esi, %eax
	orl	-24(%ebp), %eax
	leal	-1069501632(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$23, %ecx
	notl	%eax
	addl	%ebx, %ecx
	andl	%ebx, %eax
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	16(%ebp), %eax
	leal	643717713(%esi,%edi), %esi
	rorl	$18, %esi
	addl	%ecx, %esi
	leal	-373897302(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %edx
	notl	%eax
	rorl	$12, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-701558691(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	38016083(%ecx,%edi), %ecx
	notl	%eax
	rorl	$23, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	12(%ebp), %eax
	leal	-660478335(%esi,%edi), %esi
	rorl	$18, %esi
	addl	%ecx, %esi
	leal	-405537848(%edx,%eax), %edx
	movl	%ebx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %edx
	notl	%eax
	rorl	$12, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	568446438(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-1019803690(%ecx,%edi), %ecx
	notl	%eax
	rorl	$23, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-187363961(%esi,%edi), %esi
	notl	%eax
	rorl	$18, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	1163531501(%edx,%edi), %edx
	notl	%eax
	rorl	$12, %edx
	andl	%esi, %eax
	addl	%esi, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-1444681467(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	-51403784(%ecx,%edi), %ecx
	notl	%eax
	rorl	$23, %ecx
	andl	%ebx, %eax
	addl	%ebx, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	1735328473(%esi,%edi), %esi
	notl	%eax
	rorl	$18, %esi
	andl	%ecx, %eax
	addl	%ecx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-1926607734(%edx,%edi), %edx
	movl	8(%ebp), %edi
	rorl	$12, %edx
	addl	%esi, %edx
	xorl	%ecx, %eax
	xorl	%edx, %eax
	leal	-378558(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-2022574463(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	1839030562(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$16, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-35309556(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$9, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-1530992060(%ebx,%eax), %ebx
	movl	12(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	1272893353(%ecx,%eax), %ecx
	movl	%edx, %eax
	xorl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-155497632(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$16, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-1094730640(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$9, %edx
	addl	%esi, %edx
	xorl	%ecx, %eax
	xorl	%edx, %eax
	leal	681279174(%ebx,%eax), %ebx
	movl	16(%ebp), %eax
	rorl	$28, %ebx
	addl	%edx, %ebx
	leal	-358537222(%ecx,%eax), %ecx
	movl	%edx, %eax
	xorl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	-722521979(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$16, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	76029189(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$9, %edx
	xorl	%ecx, %eax
	addl	%esi, %edx
	xorl	%edx, %eax
	leal	-640364487(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%esi, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-421815835(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%ebx, %ecx
	xorl	%ecx, %eax
	leal	530742520(%esi,%eax), %esi
	movl	%ecx, %eax
	rorl	$16, %esi
	xorl	%ebx, %eax
	addl	%ecx, %esi
	xorl	%esi, %eax
	leal	-995338651(%edx,%eax), %edx
	movl	16(%ebp), %eax
	rorl	$9, %edx
	addl	%esi, %edx
	leal	-198630844(%ebx,%eax), %ebx
	movl	%ecx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	1126891415(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-1416354905(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$17, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-57434055(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	1700485571(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-1894986606(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-1051523(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$17, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	-2054922799(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%esi, %edx
	orl	%edx, %eax
	xorl	%esi, %eax
	leal	1873313359(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-30611744(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%ebx, %ecx
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	-1560198380(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$17, %esi
	notl	%eax
	addl	%ecx, %esi
	orl	%esi, %eax
	xorl	%ecx, %eax
	leal	1309151649(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%esi, %edx
	movl	%edx, -20(%ebp)
	movl	12(%ebp), %edx
	orl	-20(%ebp), %eax
	leal	-145523070(%ebx,%edx), %ebx
	xorl	%esi, %eax
	addl	%eax, %ebx
	movl	%esi, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	-20(%ebp), %ebx
	orl	%ebx, %eax
	xorl	-20(%ebp), %eax
	leal	-1120210379(%ecx,%eax), %ecx
	movl	-20(%ebp), %eax
	rorl	$22, %ecx
	addl	%ebx, %ecx
	movl	%ecx, -16(%ebp)
	notl	%eax
	orl	%ecx, %eax
	xorl	%ebx, %eax
	leal	718787259(%esi,%eax), %esi
	rorl	$17, %esi
	addl	%ecx, %esi
	testl	%edi, %edi
	je	.L119
.L117:
	leal	-1034786161(%ebx), %eax
	movl	-20(%ebp), %edx
	notl	%ebx
	orl	%esi, %ebx
	xorl	-16(%ebp), %ebx
	movl	%eax, 8(%edi)
	leal	-343485551(%edx,%ebx), %eax
	rorl	$11, %eax
	leal	1013567884(%esi,%eax), %eax
	movl	%eax, 12(%edi)
	leal	-1386958736(%esi), %eax
	movl	%eax, 16(%edi)
	movl	-16(%ebp), %eax
	addl	$68628098, %eax
	movl	%eax, 20(%edi)
	addl	$28, %esp
	movl	%edi, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L119:
	call	H_New
	movl	%eax, %edi
	jmp	.L117
	.size	_Hf_FromUInt, .-_Hf_FromUInt
	.p2align 4,,15
.globl Hf_FromUnsignedInt
	.type	Hf_FromUnsignedInt, @function
Hf_FromUnsignedInt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	12(%ebp), %eax
	movl	$699, 8(%esp)
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_Hf_FromUInt
	leave
	ret
	.size	Hf_FromUnsignedInt, .-Hf_FromUnsignedInt
	.p2align 4,,15
.globl Hf_FromInt
	.type	Hf_FromInt, @function
Hf_FromInt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	12(%ebp), %eax
	testl	%eax, %eax
	jle	.L123
	movl	$699, 8(%esp)
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_Hf_FromUInt
	leave
	ret
	.p2align 4,,7
	.p2align 3
.L123:
	movl	$694, 8(%esp)
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_Hf_FromUInt
	leave
	ret
	.size	Hf_FromInt, .-Hf_FromInt
	.p2align 4,,15
.globl Hf_Combine
	.type	Hf_Combine, @function
Hf_Combine:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$60, %esp
	movl	12(%ebp), %eax
	movl	16(%ebp), %edx
	movl	8(%eax), %ecx
	movl	12(%eax), %edi
	movl	%ecx, -60(%ebp)
	movl	16(%eax), %ecx
	movl	20(%eax), %eax
	movl	%edi, -56(%ebp)
	movl	8(%edx), %edi
	movl	%ecx, -52(%ebp)
	movl	16(%edx), %ecx
	movl	%eax, -48(%ebp)
	movl	12(%edx), %eax
	movl	20(%edx), %edx
	movl	%edi, -44(%ebp)
	movl	-56(%ebp), %edi
	movl	%ecx, -36(%ebp)
	movl	-60(%ebp), %ecx
	movl	%eax, -40(%ebp)
	movl	%edx, -32(%ebp)
	addl	$530616420, %ecx
	rorl	$25, %ecx
	addl	$1991992300, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edx
	notl	%eax
	andl	$1991992300, %edx
	andl	$-1636262282, %eax
	orl	%edx, %eax
	leal	-371597807(%edi,%eax), %esi
	rorl	$20, %esi
	addl	%ecx, %esi
	movl	%esi, %eax
	movl	%esi, %edx
	notl	%eax
	andl	%ecx, %edx
	andl	$1991992300, %eax
	orl	%edx, %eax
	movl	-52(%ebp), %edx
	leal	-1030156463(%edx,%eax), %edi
	rorl	$15, %edi
	addl	%esi, %edi
	movl	%edi, %eax
	movl	%edi, %edx
	notl	%eax
	andl	%esi, %edx
	andl	%ecx, %eax
	orl	%edx, %eax
	movl	-48(%ebp), %edx
	leal	947466970(%edx,%eax), %eax
	rorl	$10, %eax
	addl	%edi, %eax
	movl	%eax, -64(%ebp)
	movl	-44(%ebp), %eax
	movl	-64(%ebp), %edx
	leal	-176418897(%eax,%ecx), %ebx
	movl	-64(%ebp), %eax
	andl	%edi, %edx
	notl	%eax
	andl	%esi, %eax
	orl	%edx, %eax
	movl	-40(%ebp), %edx
	addl	%eax, %ebx
	rorl	$25, %ebx
	addl	-64(%ebp), %ebx
	leal	1200080426(%edx,%esi), %esi
	movl	-64(%ebp), %edx
	movl	%ebx, %eax
	notl	%eax
	andl	%ebx, %edx
	andl	%edi, %eax
	orl	%edx, %eax
	addl	%eax, %esi
	movl	-36(%ebp), %eax
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %edx
	leal	-1473231341(%eax,%edi), %ecx
	movl	%esi, %eax
	movl	-64(%ebp), %edi
	notl	%eax
	andl	%ebx, %edx
	andl	-64(%ebp), %eax
	orl	%edx, %eax
	addl	%eax, %ecx
	movl	-32(%ebp), %eax
	rorl	$15, %ecx
	addl	%esi, %ecx
	leal	-45705983(%eax,%edi), %edx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	addl	%eax, %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1770035416(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1958414417(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-42063(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-1990404162(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1804603682(%ebx,%eax), %ebx
	rorl	$25, %ebx
	addl	%edx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-40341101(%esi,%eax), %esi
	rorl	$20, %esi
	addl	%ebx, %esi
	movl	%esi, %edi
	notl	%edi
	movl	%edi, -28(%ebp)
	movl	%edi, %eax
	movl	%esi, %edi
	andl	%ebx, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1502002290(%ecx,%eax), %ecx
	rorl	$15, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	movl	%eax, -24(%ebp)
	andl	%ebx, %eax
	orl	%edi, %eax
	movl	-56(%ebp), %edi
	leal	1236535329(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%ecx, %edx
	andl	%ecx, -28(%ebp)
	movl	%edx, %eax
	andl	%esi, %eax
	orl	-28(%ebp), %eax
	leal	-165796510(%ebx,%edi), %ebx
	andl	%edx, -24(%ebp)
	addl	%eax, %ebx
	movl	-36(%ebp), %eax
	rorl	$27, %ebx
	addl	%edx, %ebx
	leal	-1069501632(%esi,%eax), %esi
	movl	%ebx, %eax
	andl	%ecx, %eax
	orl	-24(%ebp), %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$23, %esi
	notl	%eax
	addl	%ebx, %esi
	andl	%ebx, %eax
	movl	%esi, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	643717713(%ecx,%edi), %ecx
	movl	-60(%ebp), %edi
	notl	%eax
	rorl	$18, %ecx
	addl	%esi, %ecx
	andl	%esi, %eax
	leal	-373897302(%edx,%edi), %edx
	movl	%ecx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-40(%ebp), %eax
	addl	%edi, %edx
	rorl	$12, %edx
	addl	%ecx, %edx
	leal	-701558691(%ebx,%eax), %ebx
	movl	%esi, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ecx, %eax
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %ebx
	notl	%eax
	rorl	$27, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	movl	%ebx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	38016083(%esi,%edi), %esi
	notl	%eax
	rorl	$23, %esi
	andl	%ebx, %eax
	addl	%ebx, %esi
	movl	%esi, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-660478335(%ecx,%edi), %ecx
	movl	-44(%ebp), %edi
	notl	%eax
	rorl	$18, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	leal	-405537848(%edx,%edi), %edx
	movl	%ecx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %edx
	notl	%eax
	rorl	$12, %edx
	andl	%ecx, %eax
	addl	%ecx, %edx
	movl	%edx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	568446438(%ebx,%edi), %ebx
	notl	%eax
	rorl	$27, %ebx
	addl	%edx, %ebx
	andl	%edx, %eax
	movl	%ebx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-48(%ebp), %eax
	leal	-1019803690(%esi,%edi), %esi
	rorl	$23, %esi
	addl	%ebx, %esi
	leal	-187363961(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$18, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	movl	%ecx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	1163531501(%edx,%edi), %edx
	notl	%eax
	rorl	$12, %edx
	andl	%ecx, %eax
	addl	%ecx, %edx
	movl	%edx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1444681467(%ebx,%edi), %ebx
	movl	-52(%ebp), %edi
	notl	%eax
	rorl	$27, %ebx
	andl	%edx, %eax
	addl	%edx, %ebx
	leal	-51403784(%esi,%edi), %esi
	movl	%ebx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	-32(%ebp), %eax
	addl	%edi, %esi
	rorl	$23, %esi
	addl	%ebx, %esi
	leal	1735328473(%ecx,%eax), %ecx
	movl	%edx, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%ebx, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$18, %ecx
	addl	%esi, %ecx
	andl	%esi, %eax
	movl	%ecx, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-1926607734(%edx,%edi), %edx
	movl	-40(%ebp), %edi
	xorl	%esi, %eax
	rorl	$12, %edx
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	-378558(%ebx,%edi), %ebx
	movl	-44(%ebp), %edi
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%ecx, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-2022574463(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	1839030562(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	xorl	%ebx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	-35309556(%edx,%eax), %edx
	movl	-56(%ebp), %eax
	rorl	$9, %edx
	addl	%ecx, %edx
	leal	-1530992060(%ebx,%eax), %ebx
	movl	%ecx, %eax
	xorl	%esi, %eax
	xorl	%edx, %eax
	addl	%eax, %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%ecx, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	1272893353(%esi,%edi), %esi
	movl	-60(%ebp), %edi
	addl	%eax, %esi
	movl	-32(%ebp), %eax
	rorl	$21, %esi
	addl	%ebx, %esi
	leal	-155497632(%ecx,%eax), %ecx
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	xorl	%ebx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	-1094730640(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$9, %edx
	xorl	%esi, %eax
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	681279174(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%ecx, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-358537222(%esi,%edi), %esi
	movl	-36(%ebp), %edi
	addl	%eax, %esi
	movl	-48(%ebp), %eax
	rorl	$21, %esi
	addl	%ebx, %esi
	leal	-722521979(%ecx,%eax), %ecx
	movl	%ebx, %eax
	xorl	%edx, %eax
	xorl	%esi, %eax
	addl	%eax, %ecx
	movl	%esi, %eax
	rorl	$16, %ecx
	xorl	%ebx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	76029189(%edx,%edi), %edx
	movl	-60(%ebp), %edi
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$9, %edx
	xorl	%esi, %eax
	addl	%ecx, %edx
	xorl	%edx, %eax
	leal	-640364487(%ebx,%eax), %ebx
	movl	%edx, %eax
	rorl	$28, %ebx
	xorl	%ecx, %eax
	addl	%edx, %ebx
	xorl	%ebx, %eax
	leal	-421815835(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$21, %esi
	xorl	%edx, %eax
	addl	%ebx, %esi
	xorl	%esi, %eax
	leal	530742520(%ecx,%eax), %ecx
	movl	-52(%ebp), %eax
	rorl	$16, %ecx
	addl	%esi, %ecx
	leal	-995338651(%edx,%eax), %edx
	movl	%esi, %eax
	xorl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$9, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	-198630844(%ebx,%edi), %ebx
	movl	-40(%ebp), %edi
	addl	%eax, %ebx
	movl	-32(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	1126891415(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1416354905(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-57434055(%edx,%edi), %edx
	movl	-56(%ebp), %edi
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	1700485571(%ebx,%eax), %ebx
	movl	-48(%ebp), %eax
	rorl	$26, %ebx
	addl	%edx, %ebx
	leal	-1894986606(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$22, %esi
	notl	%eax
	addl	%ebx, %esi
	orl	%esi, %eax
	xorl	%ebx, %eax
	leal	-1051523(%ecx,%eax), %ecx
	movl	%ebx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-2054922799(%edx,%edi), %edx
	addl	%eax, %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	orl	%edx, %eax
	xorl	%ecx, %eax
	leal	1873313359(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	%edx, %ebx
	orl	%ebx, %eax
	xorl	%edx, %eax
	leal	-30611744(%esi,%eax), %esi
	movl	-36(%ebp), %eax
	rorl	$22, %esi
	addl	%ebx, %esi
	leal	-1560198380(%ecx,%eax), %ecx
	movl	%edx, %eax
	notl	%eax
	orl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	movl	%ebx, %eax
	rorl	$17, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	1309151649(%edx,%eax), %edx
	movl	%esi, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ecx, %edx
	movl	%edx, -20(%ebp)
	movl	-44(%ebp), %edx
	orl	-20(%ebp), %eax
	leal	-145523070(%ebx,%edx), %ebx
	xorl	%ecx, %eax
	addl	%eax, %ebx
	movl	%ecx, %eax
	rorl	$26, %ebx
	notl	%eax
	addl	-20(%ebp), %ebx
	orl	%ebx, %eax
	xorl	-20(%ebp), %eax
	leal	-1120210379(%esi,%eax), %esi
	rorl	$22, %esi
	addl	%ebx, %esi
	movl	%esi, -16(%ebp)
	movl	-52(%ebp), %edi
	movl	-20(%ebp), %eax
	leal	718787259(%ecx,%edi), %ecx
	movl	8(%ebp), %edi
	notl	%eax
	orl	%esi, %eax
	xorl	%ebx, %eax
	addl	%eax, %ecx
	rorl	$17, %ecx
	testl	%edi, %edi
	leal	(%ecx,%esi), %esi
	je	.L130
.L128:
	leal	821945717(%ebx), %eax
	movl	-20(%ebp), %edx
	notl	%ebx
	orl	%esi, %ebx
	xorl	-16(%ebp), %ebx
	movl	%eax, 8(%edi)
	leal	-343485551(%edx,%ebx), %eax
	rorl	$11, %eax
	leal	1991992300(%esi,%eax), %eax
	movl	%eax, 12(%edi)
	leal	-1636262282(%esi), %eax
	movl	%eax, 16(%edi)
	movl	-16(%ebp), %eax
	addl	$17966779, %eax
	movl	%eax, 20(%edi)
	addl	$60, %esp
	movl	%edi, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L130:
	call	H_New
	movl	%eax, %edi
	jmp	.L128
	.size	Hf_Combine, .-Hf_Combine
	.p2align 4,,15
.globl Hf_FromHashObject
	.type	Hf_FromHashObject, @function
Hf_FromHashObject:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$44, %esp
	movl	12(%ebp), %eax
	movl	8(%eax), %edx
	movl	12(%eax), %edi
	movl	%edx, -44(%ebp)
	movl	-44(%ebp), %esi
	movl	16(%eax), %edx
	movl	20(%eax), %eax
	movl	%edi, -40(%ebp)
	subl	$798693669, %esi
	rorl	$25, %esi
	leal	-943578591(%esi), %edi
	movl	%eax, -32(%ebp)
	movl	%edi, %eax
	movl	%edx, -36(%ebp)
	notl	%eax
	movl	%edi, %edx
	andl	$-943578591, %edx
	andl	$317033537, %eax
	orl	%edx, %eax
	movl	-40(%ebp), %edx
	leal	-845621654(%edx,%eax), %ecx
	rorl	$20, %ecx
	addl	%edi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edx
	notl	%eax
	andl	%edi, %edx
	andl	$-943578591, %eax
	orl	%edx, %eax
	movl	-36(%ebp), %edx
	leal	923139356(%edx,%eax), %ebx
	rorl	$15, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edx
	notl	%eax
	andl	%ecx, %edx
	andl	%edi, %eax
	movl	-32(%ebp), %edi
	orl	%edx, %eax
	leal	-1988103921(%edi,%eax), %edx
	rorl	$10, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	-1119997488(%esi,%eax), %esi
	rorl	$25, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	1200080426(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1473231341(%ebx,%eax), %ebx
	rorl	$15, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%ecx, %edi
	orl	%edi, %eax
	leal	-45705983(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	1770035416(%esi,%eax), %esi
	rorl	$25, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-1958414417(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-42063(%ebx,%eax), %ebx
	rorl	$15, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %eax
	movl	%ebx, %edi
	notl	%eax
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	-1990404162(%edx,%eax), %edx
	rorl	$10, %edx
	addl	%ebx, %edx
	movl	%edx, %eax
	movl	%edx, %edi
	notl	%eax
	andl	%ebx, %edi
	andl	%ecx, %eax
	orl	%edi, %eax
	leal	1804603682(%esi,%eax), %esi
	rorl	$25, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	movl	%esi, %edi
	notl	%eax
	andl	%edx, %edi
	andl	%ebx, %eax
	orl	%edi, %eax
	leal	-40341101(%ecx,%eax), %ecx
	rorl	$20, %ecx
	addl	%esi, %ecx
	movl	%ecx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %edi
	movl	%eax, -28(%ebp)
	andl	%edx, %eax
	orl	%edi, %eax
	leal	-1502002290(%ebx,%eax), %ebx
	rorl	$15, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %edi
	notl	%edi
	movl	%edi, -24(%ebp)
	movl	%edi, %eax
	movl	%ebx, %edi
	andl	%ecx, %edi
	andl	%esi, %eax
	orl	%edi, %eax
	leal	1236535329(%edx,%eax), %edx
	movl	-40(%ebp), %eax
	rorl	$10, %edx
	addl	%ebx, %edx
	andl	%ebx, -28(%ebp)
	leal	-165796510(%esi,%eax), %esi
	movl	%edx, %eax
	andl	%ecx, %eax
	orl	-28(%ebp), %eax
	andl	%edx, -24(%ebp)
	addl	%eax, %esi
	rorl	$27, %esi
	addl	%edx, %esi
	movl	%esi, %eax
	andl	%ebx, %eax
	orl	-24(%ebp), %eax
	leal	-1069501632(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$23, %ecx
	notl	%eax
	addl	%esi, %ecx
	andl	%esi, %eax
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	643717713(%ebx,%edi), %ebx
	movl	-44(%ebp), %edi
	notl	%eax
	rorl	$18, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	leal	-373897302(%edx,%edi), %edx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	addl	%edi, %edx
	notl	%eax
	rorl	$12, %edx
	addl	%ebx, %edx
	andl	%ebx, %eax
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-701558691(%esi,%edi), %esi
	notl	%eax
	rorl	$27, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	leal	38016083(%ecx,%edi), %ecx
	notl	%eax
	rorl	$23, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	-660478335(%ebx,%edi), %ebx
	notl	%eax
	rorl	$18, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	-405537848(%edx,%edi), %edx
	notl	%eax
	rorl	$12, %edx
	andl	%ebx, %eax
	addl	%ebx, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	568446438(%esi,%edi), %esi
	notl	%eax
	rorl	$27, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	-32(%ebp), %eax
	leal	-1019803690(%ecx,%edi), %ecx
	rorl	$23, %ecx
	addl	%esi, %ecx
	leal	-187363961(%ebx,%eax), %ebx
	movl	%edx, %eax
	movl	%ecx, %edi
	notl	%eax
	andl	%esi, %eax
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	addl	%edi, %ebx
	notl	%eax
	rorl	$18, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ecx, %eax
	leal	1163531501(%edx,%edi), %edx
	notl	%eax
	rorl	$12, %edx
	andl	%ebx, %eax
	addl	%ebx, %edx
	movl	%edx, %edi
	andl	%ecx, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-1444681467(%esi,%edi), %esi
	movl	-36(%ebp), %edi
	notl	%eax
	rorl	$27, %esi
	andl	%edx, %eax
	addl	%edx, %esi
	leal	-51403784(%ecx,%edi), %ecx
	movl	%esi, %edi
	andl	%ebx, %edi
	orl	%eax, %edi
	movl	%edx, %eax
	addl	%edi, %ecx
	notl	%eax
	rorl	$23, %ecx
	andl	%esi, %eax
	addl	%esi, %ecx
	movl	%ecx, %edi
	andl	%edx, %edi
	orl	%eax, %edi
	movl	%esi, %eax
	leal	1735328473(%ebx,%edi), %ebx
	notl	%eax
	rorl	$18, %ebx
	andl	%ecx, %eax
	addl	%ecx, %ebx
	movl	%ebx, %edi
	andl	%esi, %edi
	orl	%eax, %edi
	movl	%ebx, %eax
	leal	-1926607734(%edx,%edi), %edx
	xorl	%ecx, %eax
	movl	-44(%ebp), %edi
	rorl	$12, %edx
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	-378558(%esi,%eax), %esi
	movl	%edx, %eax
	rorl	$28, %esi
	addl	%edx, %esi
	xorl	%ebx, %eax
	xorl	%esi, %eax
	leal	-2022574463(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	1839030562(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$16, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	-35309556(%edx,%eax), %edx
	movl	-40(%ebp), %eax
	rorl	$9, %edx
	addl	%ebx, %edx
	leal	-1530992060(%esi,%eax), %esi
	movl	%ebx, %eax
	xorl	%ecx, %eax
	xorl	%edx, %eax
	addl	%eax, %esi
	movl	%edx, %eax
	rorl	$28, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	1272893353(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	-155497632(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$16, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	-1094730640(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$9, %edx
	xorl	%ecx, %eax
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	681279174(%esi,%eax), %esi
	movl	%edx, %eax
	rorl	$28, %esi
	addl	%edx, %esi
	xorl	%ebx, %eax
	xorl	%esi, %eax
	leal	-358537222(%ecx,%edi), %ecx
	movl	-36(%ebp), %edi
	addl	%eax, %ecx
	movl	-32(%ebp), %eax
	rorl	$21, %ecx
	addl	%esi, %ecx
	leal	-722521979(%ebx,%eax), %ebx
	movl	%esi, %eax
	xorl	%edx, %eax
	xorl	%ecx, %eax
	addl	%eax, %ebx
	movl	%ecx, %eax
	rorl	$16, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	76029189(%edx,%eax), %edx
	movl	%ebx, %eax
	rorl	$9, %edx
	xorl	%ecx, %eax
	addl	%ebx, %edx
	xorl	%edx, %eax
	leal	-640364487(%esi,%eax), %esi
	movl	%edx, %eax
	rorl	$28, %esi
	xorl	%ebx, %eax
	addl	%edx, %esi
	xorl	%esi, %eax
	leal	-421815835(%ecx,%eax), %ecx
	movl	%esi, %eax
	rorl	$21, %ecx
	xorl	%edx, %eax
	addl	%esi, %ecx
	xorl	%ecx, %eax
	leal	530742520(%ebx,%eax), %ebx
	movl	%ecx, %eax
	rorl	$16, %ebx
	xorl	%esi, %eax
	addl	%ecx, %ebx
	xorl	%ebx, %eax
	leal	-995338651(%edx,%edi), %edx
	movl	-32(%ebp), %edi
	addl	%eax, %edx
	movl	-44(%ebp), %eax
	rorl	$9, %edx
	addl	%ebx, %edx
	leal	-198630844(%esi,%eax), %esi
	movl	%ecx, %eax
	notl	%eax
	orl	%edx, %eax
	xorl	%ebx, %eax
	addl	%eax, %esi
	movl	%ebx, %eax
	rorl	$26, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	1126891415(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-1416354905(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$17, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	-57434055(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	1700485571(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$26, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-1894986606(%ecx,%edi), %ecx
	addl	%eax, %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-1051523(%ebx,%eax), %ebx
	movl	-40(%ebp), %eax
	rorl	$17, %ebx
	addl	%ecx, %ebx
	leal	-2054922799(%edx,%eax), %edx
	movl	%esi, %eax
	notl	%eax
	orl	%ebx, %eax
	xorl	%ecx, %eax
	addl	%eax, %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	1873313359(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$26, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	leal	-30611744(%ecx,%eax), %ecx
	movl	%edx, %eax
	rorl	$22, %ecx
	notl	%eax
	addl	%esi, %ecx
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	-1560198380(%ebx,%eax), %ebx
	movl	%esi, %eax
	rorl	$17, %ebx
	notl	%eax
	addl	%ecx, %ebx
	orl	%ebx, %eax
	xorl	%ecx, %eax
	leal	1309151649(%edx,%eax), %edx
	movl	%ecx, %eax
	rorl	$11, %edx
	notl	%eax
	addl	%ebx, %edx
	orl	%edx, %eax
	xorl	%ebx, %eax
	leal	-145523070(%esi,%eax), %esi
	movl	%ebx, %eax
	rorl	$26, %esi
	notl	%eax
	addl	%edx, %esi
	orl	%esi, %eax
	xorl	%edx, %eax
	movl	%edx, -20(%ebp)
	leal	-1120210379(%ecx,%eax), %ecx
	movl	-20(%ebp), %eax
	movl	-36(%ebp), %edx
	rorl	$22, %ecx
	addl	%esi, %ecx
	movl	8(%ebp), %edi
	movl	%ecx, -16(%ebp)
	notl	%eax
	orl	%ecx, %eax
	xorl	%esi, %eax
	leal	718787259(%ebx,%edx), %ebx
	addl	%eax, %ebx
	rorl	$17, %ebx
	addl	%ecx, %ebx
	testl	%edi, %edi
	je	.L135
.L133:
	leal	-701946546(%esi), %eax
	movl	-20(%ebp), %edx
	notl	%esi
	orl	%ebx, %esi
	xorl	-16(%ebp), %esi
	movl	%eax, 8(%edi)
	leal	-343485551(%edx,%esi), %eax
	rorl	$11, %eax
	leal	-943578591(%ebx,%eax), %eax
	movl	%eax, 12(%edi)
	leal	317033537(%ebx), %eax
	movl	%eax, 16(%edi)
	movl	-16(%ebp), %eax
	subl	$456057068, %eax
	movl	%eax, 20(%edi)
	addl	$44, %esp
	movl	%edi, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,7
	.p2align 3
.L135:
	call	H_New
	movl	%eax, %edi
	jmp	.L133
	.size	Hf_FromHashObject, .-Hf_FromHashObject
	.p2align 4,,15
.globl Hf_FromCharBuffer
	.type	Hf_FromCharBuffer, @function
Hf_FromCharBuffer:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	movl	16(%ebp), %eax
	movl	$943663505, -16(%ebp)
	movl	$518760494, -12(%ebp)
	movl	$1677671752, -8(%ebp)
	movl	%eax, 8(%esp)
	movl	12(%ebp), %eax
	movl	$-11775187, -4(%ebp)
	movl	%eax, 4(%esp)
	leal	-16(%ebp), %eax
	movl	%eax, (%esp)
	call	md5_run
	movl	8(%ebp), %edx
	testl	%edx, %edx
	je	.L140
.L138:
	movl	-16(%ebp), %eax
	movl	%eax, 8(%edx)
	movl	-12(%ebp), %eax
	movl	%eax, 12(%edx)
	movl	-8(%ebp), %eax
	movl	%eax, 16(%edx)
	movl	-4(%ebp), %eax
	movl	%eax, 20(%edx)
	movl	%edx, %eax
	leave
	ret
	.p2align 4,,7
	.p2align 3
.L140:
	call	H_New
	movl	%eax, %edx
	jmp	.L138
	.size	Hf_FromCharBuffer, .-Hf_FromCharBuffer
	.p2align 4,,15
.globl Hf_FromString
	.type	Hf_FromString, @function
Hf_FromString:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	12(%ebp), %ebx
	movl	%ebx, (%esp)
	call	strlen
	movl	%ebx, 4(%esp)
	movl	%eax, 8(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	Hf_FromCharBuffer
	addl	$20, %esp
	popl	%ebx
	popl	%ebp
	ret
	.size	Hf_FromString, .-Hf_FromString
.globl HashObject_TypeStruct
	.data
	.align 4
	.type	HashObject_TypeStruct, @object
	.size	HashObject_TypeStruct, 16
HashObject_TypeStruct:
	.long	.LC11
	.long	free
	.long	0
	.long	0
	.section	.rodata
	.align 32
	.type	_bit_masks, @object
	.size	_bit_masks, 132
_bit_masks:
	.long	0
	.long	1
	.long	3
	.long	7
	.long	15
	.long	31
	.long	63
	.long	127
	.long	255
	.long	511
	.long	1023
	.long	2047
	.long	4095
	.long	8191
	.long	16383
	.long	32767
	.long	65535
	.long	131071
	.long	262143
	.long	524287
	.long	1048575
	.long	2097151
	.long	4194303
	.long	8388607
	.long	16777215
	.long	33554431
	.long	67108863
	.long	134217727
	.long	268435455
	.long	536870911
	.long	1073741823
	.long	2147483647
	.long	-1
	.local	map.6880
	.comm	map.6880,256,32
	.type	map.6834, @object
	.size	map.6834, 16
map.6834:
	.byte	48
	.byte	49
	.byte	50
	.byte	51
	.byte	52
	.byte	53
	.byte	54
	.byte	55
	.byte	56
	.byte	57
	.byte	97
	.byte	98
	.byte	99
	.byte	100
	.byte	101
	.byte	102
	.type	__func__.3161, @object
	.size	__func__.3161, 15
__func__.3161:
	.string	"H_ENSUREHASHED"
	.ident	"GCC: (Ubuntu 4.3.2-1ubuntu12) 4.3.2"
	.section	.note.GNU-stack,"",@progbits
