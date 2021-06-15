![alt text](https://siodb.io/wp-content/uploads/2020/05/SIODB_Logo_Editable_half_landscape_small_logo_site.png)

## Deliver safer and faster with Siodb, a Swiss-knife database to eliminate database complexity

- **Fully automated:** Automatic primary key, data sizing, indexing, defragmentation, etc
- **Fully encrypted:** Your data always encrypted at rest and in-transit
- **Easy integration:** The best of both worlds with REST and SQL combined

```bash
curl -X POST \
-d '[
      {
         "EMAIL" : "dwain.jonhson@gmail.com",
         "FIRST_NAME" : "Dwain",
         "LAST_NAME" : "Jonhson",
         "USERNAME" : "dwainjonhson"
      }
]' \
https://${USER}:${TOKEN}@localhost:50443/database/my_app/tables/users/rows
```

```bash
curl https://${USER}:${TOKEN}@localhost:50443/query?q='select * from my_app.users;'
```

```json
{
   "status" : 200,
   "rows" : [
      {
         "TRID" : 1,
         "EMAIL" : "jason.statham@gmail.com",
         "FIRST_NAME" : "Jason",
         "LAST_NAME" : "Statham",
         "USERNAME" : "jasonstatham"
      },
      {
         "TRID" : 2,
         "EMAIL" : "dwain.jonhson@gmail.com",
         "FIRST_NAME" : "Dwain",
         "LAST_NAME" : "Jonhson",
         "USERNAME" : "dwainjonhson"
      }
   ]
}
```

```sql
siocli> select * from my_app.users;

TRID   EMAIL                       FIRST_NAME     LAST_NAME     USERNAME
------ --------------------------- -------------- ------------- --------------
     1 jason.statham@gmail.com     Jason          Statham       jasonstatham
     2 dwain.jonhson@gmail.com     Dwain          Jonhson       dwainjonhson

Row(s): 2

Elapsed time: 15 ms.
```

## Help Siodb

Your support is welcomed. One-click on the star 🟊 on the top right corner of this page ☝☝ is a lot for us!

## Get started for free with Siodb as an API in your serverless platform

<a href="https://datahub.siodb.io/" target="_blank">
<img src="https://datahub.siodb.io/static/siodb_data_hub_open_graph_x1200.jpg" width="200" />
</a>

## Contributing

- Go to the contribution page 👉 [Here](CONTRIBUTING.md).

## Links

- Official website of the project 👉 [here](https://siodb.io).
- Report your issue 👉 [here](https://github.com/siodb/siodb/issues/new).
- Ask a question 👉 [here](https://stackoverflow.com/questions/tagged/siodb).
- Siodb Slack space 👉 [here](https://join.slack.com/t/siodb-squad/shared_invite/zt-e766wbf9-IfH9WiGlUpmRYlwCI_28ng).

## Social

- [Twitter](https://twitter.com/Sio_db)
- [Linkedin](https://www.linkedin.com/company/siodb)

## License

Siodb is free, and the source is available under the AGPL v3. Siodb uses
bundled dependencies for which you can find the license in the NOTICE file
in this repository's top-level directory.
