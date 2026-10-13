import boto3
session = boto3.Session()
s3_client = session.client('s3')
b = s3_client.list_buckets()
for item in b['Buckets']:
    print(item['Name'])




LabIDEURL	https://d2p1u3u9qknk2g.cloudfront.net/?folder=/home/ec2-user/environment
LabIDEPassword	a9194b30

LabIDEURL	https://d21emm1poxpekz.cloudfront.net/?folder=/home/ec2-user/environment
LabIDEPassword	11713090

aws s3api create-bucket --bucket sm-2022-08-26-s3site --region us-east-1


"Location": "/rih-2022-08-26-s3site",
    "BucketArn": "arn:aws:s3:::rih-2022-08-26-s3site"