from scapy.all import *
import base64


def analyzePcap(filepath):

    s1 = PcapReader(filepath)

    # data 是以太网 数据包
    data = s1.read_packet()
    ip_packet = data.payload
    icmp_packet = ip_packet.payload
    payload = icmp_packet.payload 
    original_payload = payload.original
    #hex_payload = original_payload.hex()

    #print(payload)     # <class 'scapy.packet.Raw'>
    print("payload: ", original_payload) # <class 'bytes'>
    #print(hex_payload)    # <class 'str'>
    
    #print(type(data.payload))  #==><class 'scapy.layers.inet.IP'>  可以使用 help(scapy.layers.inet.IP) 查看帮助文档
    
    
    
    result = base64.b64decode(original_payload)
    print("decode: ", result)


analyzePcap('~testtwo.pcap')




