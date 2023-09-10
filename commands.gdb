	echo starting from target remote: ca\n
	echo starting from monitor reset: cc\n

define ca
	symbol-file bao.debug
	add-symbol-file baremetal.debug
	target remote:3333
	monitor reset halt
	load fw_payload.elf
end

define cc
	monitor reset halt
	load fw_payload.elf
end

