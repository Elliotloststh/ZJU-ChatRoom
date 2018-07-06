#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define Register_Request 1
#define Register_Return 2
#define Register_Extra_Request 3
#define Register_Extra_Return 4
#define Login_Request 5
#define Login_Return 6
#define ChangeState_Request 7
#define ChangeState_Return 8
#define FriendList_Request 9
#define FriendList_Return 10
#define UserInfo_Request 11
#define UserInfo_Return 12
#define PublicRecord_Request 13
#define PublicRecord_Return 14
#define PrivateRecord_Request 15
#define PrivateRecord_Return 16
#define PublicMessage_Request 17
#define PublicMessage_Return 18
#define PrivateMessage_Request 19
#define PrivateMessage_Return 20
#define CheckAlive 21
#define RespondAlive 22
#define CheckOnline_Request 23
#define CheckOnline_Return 24
#define AddFriend_Request 25
#define AddFriend_Return 26
#define CheckApplication_Request 27
#define CheckApplication_Return 28
#define DeleteFriend_Request 29
#define DeleteFriend_Return 30
#define UpdateFriendList 31
#define AgreeApplication_Request 32
#define AgreeApplication_Return 33
#define UpdateApplication 34


#define ASCII 0
#define USER_ID_Len 20
#define PASSWD_LEN 20

#endif
